#include "DataFrame.hpp"
#include <OpenXLSX.hpp>
#include <arrow/util/cancel.h>
#include <parquet/exception.h>
#include <arrow/io/api.h>
#include <utf8.h>
#include <thread>
#include <mutex>
#include <variant>

namespace TinaToolBox {
    DataFrame::DataFrame(std::shared_ptr<arrow::Table> table): table_(std::move(table)) {
   
    }

    DataFrame DataFrame::fromExcel(const std::string& filePath) {
        OpenXLSX::XLDocument doc;
        try {
            doc.open(filePath);
            auto wbk = doc.workbook();
            if (wbk.sheetCount() == 0) {
                throw std::runtime_error("Excel file contains no sheets");
            }
            auto wks = wbk.sheet(1).get<OpenXLSX::XLWorksheet>();

            size_t maxCol = wks.columnCount();
            size_t maxRow = wks.rowCount() - 1;

            // 预分配内存
            std::vector<std::shared_ptr<arrow::Field>> fields(maxCol);
            std::vector<std::shared_ptr<arrow::ArrayBuilder>> builders(maxCol);
            std::vector<std::vector<OpenXLSX::XLCellValue>> columnData(maxCol);

            // 第一步：快速读取所有数据到内存（按列）
            for (size_t col = 0; col < maxCol; ++col) {
                columnData[col].reserve(maxRow + 1);
                for (size_t row = 1; row <= maxRow + 1; ++row) {
                    columnData[col].push_back(wks.cell(OpenXLSX::XLCellReference(row, col + 1)).value());
                }
            }

            // 关闭Excel文件，避免资源占用
            doc.close();

            // 使用线程池并行处理每列数据
            auto& pool = getThreadPool();
            std::vector<std::future<std::pair<size_t, std::shared_ptr<arrow::Array>>>> futures;
            std::mutex mutex;

            for (size_t col = 0; col < maxCol; ++col) {
                futures.push_back(pool.submit([col, &columnData, &fields]() {
                    const auto& colValues = columnData[col];
                    
                    // 获取列名
                    std::string columnName;
                    if (colValues[0].type() == OpenXLSX::XLValueType::Empty) {
                        columnName = "Column_" + std::to_string(col + 1);
                    } else {
                        columnName = colValues[0].get<std::string>();
                    }

                    // 确定列类型
                    bool hasNonNumeric = false;
                    bool hasDecimal = false;
                    
                    for (size_t i = 1; i < colValues.size() && !hasNonNumeric; ++i) {
                        auto cellType = colValues[i].type();
                        if (cellType != OpenXLSX::XLValueType::Empty) {
                            switch (cellType) {
                                case OpenXLSX::XLValueType::Float:
                                    hasDecimal = true;
                                    break;
                                case OpenXLSX::XLValueType::Integer:
                                    break;
                                default:
                                    hasNonNumeric = true;
                                    break;
                            }
                        }
                    }

                    // 创建builder和处理数据
                    std::shared_ptr<arrow::ArrayBuilder> builder;
                    std::shared_ptr<arrow::DataType> type;
                    
                    if (hasNonNumeric) {
                        type = arrow::utf8();
                        builder = std::make_shared<arrow::StringBuilder>();
                    } else if (hasDecimal) {
                        type = arrow::float64();
                        builder = std::make_shared<arrow::DoubleBuilder>();
                    } else {
                        type = arrow::int64();
                        builder = std::make_shared<arrow::Int64Builder>();
                    }

                    fields[col] = arrow::field(columnName, type);

                    // 处理列数据
                    std::vector<std::variant<std::string, double, int64_t>> batch;
                    batch.reserve(colValues.size() - 1);

                    for (size_t i = 1; i < colValues.size(); ++i) {
                        const auto& cellValue = colValues[i];
                        if (cellValue.type() != OpenXLSX::XLValueType::Empty) {
                            try {
                                if (hasNonNumeric) {
                                    batch.emplace_back(cellValue.get<std::string>());
                                } else if (hasDecimal) {
                                    batch.emplace_back(cellValue.get<double>());
                                } else {
                                    batch.emplace_back(cellValue.get<int64_t>());
                                }
                            } catch (...) {
                                batch.emplace_back(std::string());
                            }
                        } else {
                            batch.emplace_back(std::string());
                        }
                    }

                    appendBatch(builder, batch, type);
                    
                    std::shared_ptr<arrow::Array> array;
                    auto status = builder->Finish(&array);
                    if (!status.ok()) {
                        throw std::runtime_error("Error finishing array builder: " + status.message());
                    }

                    return std::make_pair(col, array);
                }));
            }

            // 收集结果
            std::vector<std::shared_ptr<arrow::Array>> arrays(maxCol);
            for (auto& future : futures) {
                auto result = future.get();
                arrays[result.first] = result.second;
            }

            return DataFrame(arrow::Table::Make(arrow::schema(fields), arrays));

        } catch (const std::exception& e) {
            throw std::runtime_error("Error processing Excel file: " + std::string(e.what()));
        }
    }

    // 辅助函数：批量追加数据
    void DataFrame::appendBatch(std::shared_ptr<arrow::ArrayBuilder>& builder,
                              const std::vector<std::variant<std::string, double, int64_t>>& batch,
                              const std::shared_ptr<arrow::DataType>& type) {
        if (type->Equals(arrow::utf8())) {
            auto string_builder = static_cast<arrow::StringBuilder*>(builder.get());
            for (const auto& value : batch) {
                if (const std::string* str = std::get_if<std::string>(&value)) {
                    auto status = string_builder->Append(*str);
                    if (!status.ok()) throw std::runtime_error(status.message());
                } else {
                    auto status = string_builder->AppendNull();
                    if (!status.ok()) throw std::runtime_error(status.message());
                }
            }
        } else if (type->Equals(arrow::float64())) {
            auto double_builder = static_cast<arrow::DoubleBuilder*>(builder.get());
            for (const auto& value : batch) {
                if (const double* d = std::get_if<double>(&value)) {
                    auto status = double_builder->Append(*d);
                    if (!status.ok()) throw std::runtime_error(status.message());
                } else {
                    auto status = double_builder->AppendNull();
                    if (!status.ok()) throw std::runtime_error(status.message());
                }
            }
        } else {
            auto int_builder = static_cast<arrow::Int64Builder*>(builder.get());
            for (const auto& value : batch) {
                if (const int64_t* i = std::get_if<int64_t>(&value)) {
                    auto status = int_builder->Append(*i);
                    if (!status.ok()) throw std::runtime_error(status.message());
                } else {
                    auto status = int_builder->AppendNull();
                    if (!status.ok()) throw std::runtime_error(status.message());
                }
            }
        }
    }

    std::vector<std::string> DataFrame::getColumnNames() const {
        if (!table_) {
            return {};
        }
        return table_->ColumnNames();
    }

    std::shared_ptr<arrow::ChunkedArray> DataFrame::getColumn(const std::string &name) const {
        if (!table_) {
            throw std::runtime_error("DataFrame is empty.");
        }
        auto colum = table_->GetColumnByName(name);
        if (!colum) {
            throw std::runtime_error("Column not found: " + name);
        }
        return colum;
    }

    arrow::Result<std::shared_ptr<arrow::RecordBatch> > DataFrame::getRow(int64_t index) const {
        if (!table_) {
            return arrow::Status::Invalid("DataFrame is empty.");
        }

        if (table_ < 0 || index >= table_->num_rows()) {
            return arrow::Status::Invalid("Row index out of range");
        }

        std::vector<std::shared_ptr<arrow::Array> > arrays;
        for (int i = 0; i < table_->num_columns(); ++i) {
            // 对于每一列，获取对应行的值
            arrays.push_back(table_->column(i)->chunk(0)->Slice(index, 1));
        }
        return arrow::RecordBatch::Make(table_->schema(), 1, arrays);
    }

    // arrow::Result<DataFrame> DataFrame::filter(const std::string &column,
    //                                     const std::shared_ptr<arrow::Scalar> &value) const {
    //     if (!table_) {
    //         return arrow::Status::Invalid("DataFrame is empty");
    //     }
    //
    //     auto col = table_->GetColumnByName(column);
    //     if (!col) {
    //         return arrow::Status::Invalid("Column not found: " + column);
    //     }
    //
    //     // 使用新的Arrow compute API
    //     ARROW_ASSIGN_OR_RAISE(auto mask,
    //         arrow::compute::CallFunction("equal", {col, value}));
    //
    //     // 使用过滤器
    //     ARROW_ASSIGN_OR_RAISE(auto filtered_table,
    //         arrow::compute::CallFunction("filter", {table_, mask}));
    //
    //     return DataFrame(filtered_table.table());
    // }

    arrow::Result<DataFrame> DataFrame::filter(const std::string &column,
                                               const std::shared_ptr<arrow::Scalar> &value,
                                               const std::string &comparison_operator) const {
        if (!table_) {
            return arrow::Status::Invalid("DataFrame is empty");
        }

        auto col = table_->GetColumnByName(column);
        if (!col) {
            return arrow::Status::Invalid("Column not found: " + column);
        }

        // 根据比较操作符选择函数
        std::string function_name = comparison_operator;
        if (function_name != "equal" && function_name != "not_equal" &&
            function_name != "less" && function_name != "less_equal" &&
            function_name != "greater" && function_name != "greater_equal") {
            return arrow::Status::Invalid("Invalid comparison operator: " + comparison_operator);
        }

        // 执行比较操作
        arrow::Result<arrow::Datum> compare_result = arrow::compute::CallFunction(function_name, {col, value});
        if (!compare_result.ok()) {
            // 直接构造 arrow::Status 对象来添加错误信息
            return arrow::Status(compare_result.status().code(),
                                 "Error performing comparison operation: " + compare_result.status().message());
        }

        // 使用过滤器
        arrow::Result<arrow::Datum> filter_result = arrow::compute::CallFunction("filter", {
            table_, compare_result.ValueOrDie()
        });

        if (!filter_result.ok()) {
            // 直接构造 arrow::Status 对象来添加错误信息
            return arrow::Status(filter_result.status().code(),
                                 "Error filtering table: " + filter_result.status().message());
        }

        return DataFrame(filter_result.ValueOrDie().table());
    }

    arrow::Result<DataFrame> DataFrame::sort(const std::string &column, bool ascending) const {
        if (!table_) {
            return arrow::Status::Invalid("DataFrame is empty");
        }

        // 创建排序选项
        arrow::compute::SortOptions options({
            arrow::compute::SortKey(column, ascending
                                                ? arrow::compute::SortOrder::Ascending
                                                : arrow::compute::SortOrder::Descending)
        });

        // 获取排序索引
        ARROW_ASSIGN_OR_RAISE(auto indices,
                              arrow::compute::CallFunction("sort_indices", {table_}, &options));

        // 使用索引重新排序表
        ARROW_ASSIGN_OR_RAISE(auto sorted_table,
                              arrow::compute::CallFunction("take", {table_, indices}));

        return DataFrame(sorted_table.table());
    }


    template<typename T>
    arrow::Result<T> DataFrame::getValue(int64_t row, const std::string &column) const {
        if (!table_) {
            return arrow::Status::Invalid("DataFrame is empty");
        }

        auto col = table_->GetColumnByName(column);
        if (!col) {
            return arrow::Status::Invalid("Column not found: " + column);
        }

        if (row < 0 || row >= col->length()) {
            return arrow::Status::Invalid("Row index out of range");
        }

        // 遍历 ChunkedArray 的所有 chunk
        int64_t current_row = row;
        for (const auto &chunk: col->chunks()) {
            if (current_row < chunk->length()) {
                // 确定列的类型
                auto type = chunk->type();

                // 根据列的类型，将 ChunkedArray 转换为相应的 Array 类型
                if (type->Equals(arrow::int64())) {
                    auto array = std::static_pointer_cast<arrow::Int64Array>(chunk);
                    if (array->IsValid(current_row)) {
                        return arrow::internal::checked_cast<T>(array->Value(current_row));
                    }
                } else if (type->Equals(arrow::float64())) {
                    auto array = std::static_pointer_cast<arrow::DoubleArray>(chunk);
                    if (array->IsValid(current_row)) {
                        return arrow::internal::checked_cast<T>(array->Value(current_row));
                    }
                } else if (type->Equals(arrow::utf8())) {
                    auto array = std::static_pointer_cast<arrow::StringArray>(chunk);
                    if (array->IsValid(current_row)) {
                        return static_cast<T>(array->GetString(current_row));
                        // StringArray 的 GetString 返回的是 std::string_view
                    }
                } else {
                    return arrow::Status::NotImplemented("Unsupported data type for getValue");
                }
                break; // 找到包含指定行的 chunk 后退出循环
            }
            current_row -= chunk->length();
        }

        return arrow::Status::Invalid("Value is null or row index is out of bounds for the chunk");
    }

    bool DataFrame::toSaveExcel(const std::string &filePath, bool forceOverwrite) const {
        if (!table_) {
            return false;
        }

        try {
            // 创建输出文件流
            auto maybe_outfile = arrow::io::FileOutputStream::Open(filePath, forceOverwrite);
            if (!maybe_outfile.ok()) {
                return false;
            }
            const auto outfile = *maybe_outfile;

            // Parquet写入选项
            parquet::WriterProperties::Builder builder;
            builder.compression(parquet::Compression::SNAPPY);
            auto properties = builder.build();

            // 写入Parquet文件
            const auto status = parquet::arrow::WriteTable(
                *table_,
                arrow::default_memory_pool(),
                outfile,
                64 * 1024,
                properties
            );

            return status.ok();
        } catch (const std::exception &e) {
            return false;
        }
    }
}
