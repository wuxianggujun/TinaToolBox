#include "DataFrame.hpp"
#include <OpenXLSX.hpp>
#include <arrow/util/cancel.h>
#include <parquet/exception.h>
#include <arrow/io/api.h>
#include <utf8.h>
#include <thread>
#include <mutex>
#include <variant>
#include <regex>

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
            auto wks = wbk.worksheet(wbk.worksheetNames().front());

            size_t maxCol = wks.columnCount();
            size_t maxRow = wks.rowCount() - 1;  // 减去标题行
            
            // 预分配内存
            std::vector<std::shared_ptr<arrow::Field>> fields(maxCol);
            std::vector<std::shared_ptr<arrow::Array>> arrays(maxCol);
            
            // 优化1：使用更大的批处理大小
            constexpr size_t BATCH_SIZE = 5000;
            
            // 优化2：预分配内存以减少重新分配
            std::vector<std::vector<std::string>> columnData(maxCol);
            for (auto& col : columnData) {
                col.reserve(maxRow);
            }
            
            // 优化3：一次性读取整行数据
            for (size_t row = 1; row <= maxRow + 1; ++row) {
                for (size_t col = 1; col <= maxCol; ++col) {
                    auto cellRef = OpenXLSX::XLCellReference(row, col);
                    const auto& cell = wks.cell(cellRef);
                    
                    if (row == 1) {
                        // 处理标题行
                        std::string columnName;
                        if (cell.value().type() == OpenXLSX::XLValueType::Empty) {
                            columnName = "Column_" + std::to_string(col);
                        } else {
                            columnName = cell.value().get<std::string>();
                        }
                        fields[col-1] = std::make_shared<arrow::Field>(columnName, arrow::utf8());
                    } else {
                        // 处理数据行
                        try {
                            if (cell.value().type() == OpenXLSX::XLValueType::Empty) {
                                columnData[col-1].push_back("");
                            } else {
                                const auto& value = cell.value();
                                columnData[col-1].push_back(value.get<std::string>());
                            }
                        } catch (...) {
                            columnData[col-1].push_back("");
                        }
                    }
                }
            }
            
            // 关闭文档，释放资源
            doc.close();
            
            // 优化4：并行处理数据转换
            auto& pool = getThreadPool();
            std::vector<std::future<void>> futures;
            futures.reserve(maxCol);  // 预分配 futures 容量
            
            for (size_t col = 0; col < maxCol; ++col) {
                futures.push_back(
                    pool.submit(
                        [col, &columnData, &fields, &arrays]() {
                            try {
                                const auto& colData = columnData[col];
                                
                                // 优化5：快速类型检测
                                bool hasNonNumeric = false;
                                bool hasDecimal = false;
                                
                                // 优化6：使用更快的类型检测方法
                                const size_t sampleSize = std::min(size_t(100), colData.size());
                                
                                for (size_t i = 0; i < sampleSize && !hasNonNumeric; ++i) {
                                    const auto& value = colData[i];
                                    if (!value.empty()) {
                                        try {
                                            size_t pos;
                                            std::stod(value, &pos);
                                            if (pos != value.length()) {
                                                hasNonNumeric = true;
                                            } else if (value.find('.') != std::string::npos) {
                                                hasDecimal = true;
                                            }
                                        } catch (...) {
                                            hasNonNumeric = true;
                                        }
                                    }
                                }

                                // 优化7：创建正确大小的builder
                                std::shared_ptr<arrow::ArrayBuilder> builder;
                                if (hasNonNumeric) {
                                    auto string_builder = std::make_shared<arrow::StringBuilder>();
                                    auto status = string_builder->Reserve(colData.size());
                                    if (!status.ok()) {
                                        throw std::runtime_error("Failed to reserve memory: " + status.ToString());
                                    }
                                    builder = string_builder;
                                } else if (hasDecimal) {
                                    auto double_builder = std::make_shared<arrow::DoubleBuilder>();
                                    auto status = double_builder->Reserve(colData.size());
                                    if (!status.ok()) {
                                        throw std::runtime_error("Failed to reserve memory: " + status.ToString());
                                    }
                                    builder = double_builder;
                                } else {
                                    auto int_builder = std::make_shared<arrow::Int64Builder>();
                                    auto status = int_builder->Reserve(colData.size());
                                    if (!status.ok()) {
                                        throw std::runtime_error("Failed to reserve memory: " + status.ToString());
                                    }
                                    builder = int_builder;
                                }
                                
                                // 优化8：预分配builder容量
                                builder->Reserve(colData.size()).ok();

                                // 优化9：批量处理数据
                                for (size_t i = 0; i < colData.size(); i += BATCH_SIZE) {
                                    size_t batchEnd = std::min(i + BATCH_SIZE, colData.size());
                                    std::vector<std::variant<std::string, double, int64_t>> batch;
                                    batch.reserve(batchEnd - i);

                                    for (size_t j = i; j < batchEnd; ++j) {
                                        const auto& value = colData[j];
                                        if (!value.empty()) {
                                            try {
                                                if (hasNonNumeric) {
                                                    batch.emplace_back(value);
                                                } else if (hasDecimal) {
                                                    batch.emplace_back(std::stod(value));
                                                } else {
                                                    batch.emplace_back(std::stoll(value));
                                                }
                                            } catch (...) {
                                                batch.emplace_back(std::string());
                                            }
                                        } else {
                                            batch.emplace_back(std::string());
                                        }
                                    }

                                    appendBatch(builder, batch, fields[col]->type());
                                }

                                std::shared_ptr<arrow::Array> array;
                                auto status = builder->Finish(&array);
                                if (!status.ok()) {
                                    throw std::runtime_error("Failed to finish array: " + status.ToString());
                                }
                                arrays[col] = array;
                            } catch (const std::exception& e) {
                                throw std::runtime_error("Error processing column " + std::to_string(col) + ": " + e.what());
                            }
                        }
                    )
                );
            }
            
            // 等待所有列处理完成
            for (auto& future : futures) {
                future.get();
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
        try {
            if (auto string_builder = dynamic_cast<arrow::StringBuilder*>(builder.get())) {
                // 字符串类型处理
                for (const auto& value : batch) {
                    if (const std::string* str = std::get_if<std::string>(&value)) {
                        arrow::Status status;
                        if (str->empty()) {
                            status = string_builder->AppendNull();
                        } else {
                            status = string_builder->Append(*str);
                        }
                        if (!status.ok()) {
                            throw std::runtime_error("Failed to append string: " + status.ToString());
                        }
                    } else {
                        auto status = string_builder->AppendNull();
                        if (!status.ok()) {
                            throw std::runtime_error("Failed to append null: " + status.ToString());
                        }
                    }
                }
            }
            else if (auto double_builder = dynamic_cast<arrow::DoubleBuilder*>(builder.get())) {
                // 浮点数类型处理
                for (const auto& value : batch) {
                    arrow::Status status;
                    if (const double* d = std::get_if<double>(&value)) {
                        status = double_builder->Append(*d);
                    } else {
                        status = double_builder->AppendNull();
                    }
                    if (!status.ok()) {
                        throw std::runtime_error("Failed to append double: " + status.ToString());
                    }
                }
            }
            else if (auto int_builder = dynamic_cast<arrow::Int64Builder*>(builder.get())) {
                // 整数类型处理
                for (const auto& value : batch) {
                    arrow::Status status;
                    if (const int64_t* i = std::get_if<int64_t>(&value)) {
                        status = int_builder->Append(*i);
                    } else {
                        status = int_builder->AppendNull();
                    }
                    if (!status.ok()) {
                        throw std::runtime_error("Failed to append integer: " + status.ToString());
                    }
                }
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Error in appendBatch: " + std::string(e.what()));
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
