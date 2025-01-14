/*
#include "DataFrame.hpp"
#include <OpenXLSX.hpp>
#include <arrow/util/cancel.h>
#include <parquet/exception.h>
#include <arrow/io/api.h>
#include <utf8.h>
#include <thread>
#include <mutex>
#include <variant>
#include <ctpl_stl.h>

namespace TinaToolBox {
    DataFrame::DataFrame(std::shared_ptr<arrow::Table> table): table_(std::move(table)) {
        builders_mutex_ = std::mutex();
        fields_mutex_ = std::mutex();
        types_mutex_ = std::mutex();
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
            std::vector<std::shared_ptr<arrow::DataType>> columnTypes(maxCol);
            std::vector<std::shared_ptr<arrow::ArrayBuilder>> builders(maxCol);
            
            // 使用多线程处理列
            const size_t num_threads = std::thread::hardware_concurrency();
            std::vector<std::thread> threads;
            std::mutex mutex;
            
            // 创建线程池
            ctpl::thread_pool p(std::thread::hardware_concurrency());

            auto processColumn = [&](size_t startCol, size_t endCol) {
                for (size_t col = startCol; col < endCol; ++col) {
                    auto cell = wks.cell(OpenXLSX::XLCellReference(1, col + 1));
                    std::string columnName;
                    
                    auto cellType = cell.value().type();
                    if (cellType == OpenXLSX::XLValueType::Empty) {
                        columnName = "Column_" + std::to_string(col + 1);
                    } else {
                        columnName = cell.value().get<std::string>();
                    }

                    // 预估类型：使用批处理
                    bool hasNonNumeric = false;
                    bool hasDecimal = false;
                    const size_t BATCH_SIZE = 500;
                    
                    for (size_t row = 2; row <= maxRow + 1; row += BATCH_SIZE) {
                        size_t batchEnd = std::min(row + BATCH_SIZE, maxRow + 1);
                        for (size_t r = row; r < batchEnd && !hasNonNumeric; ++r) {
                            auto sampleCell = wks.cell(OpenXLSX::XLCellReference(r, col + 1));
                            auto sampleType = sampleCell.value().type();
                            
                            if (sampleType != OpenXLSX::XLValueType::Empty) {
                                switch (sampleType) {
                                    case OpenXLSX::XLValueType::Integer:
                                        break;
                                    case OpenXLSX::XLValueType::Float:
                                        hasDecimal = true;
                                        break;
                                    default:
                                        hasNonNumeric = true;
                                        break;
                                }
                            }
                        }
                    }

                    std::shared_ptr<arrow::DataType> type;
                    std::shared_ptr<arrow::ArrayBuilder> builder;
                    std::string columnName;
                    
                    {
                        std::lock_guard<std::mutex> lock(types_mutex_);
                        // 类型确定和builder创建
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
                    }

                    // 批量处理数据
                    std::vector<std::variant<std::string, double, int64_t>> batch;
                    batch.reserve(BATCH_SIZE);
                    
                    for (size_t row = 2; row <= maxRow + 1; ++row) {
                        auto cell_assignable = wks.cell(OpenXLSX::XLCellReference(row, col + 1));
                        auto cell_type = cell_assignable.value().type();

                        if (cell_type != OpenXLSX::XLValueType::Empty) {
                            try {
                                if (type->Equals(arrow::utf8())) {
                                    batch.emplace_back(cell_assignable.value().get<std::string>());
                                } else if (type->Equals(arrow::float64())) {
                                    batch.emplace_back(cell_assignable.value().get<double>());
                                } else {
                                    batch.emplace_back(cell_assignable.value().get<int64_t>());
                                }
                            } catch (...) {
                                batch.emplace_back(std::string());
                            }
                        } else {
                            batch.emplace_back(std::string());
                        }

                        if (batch.size() == BATCH_SIZE) {
                            std::lock_guard<std::mutex> lock(mutex);
                            appendBatch(builder, batch, type);  // 去掉 DataFrame:: 前缀
                            batch.clear();
                            batch.shrink_to_fit();
                        }
                    }

                    if (!batch.empty()) {
                        std::lock_guard<std::mutex> lock(mutex);
                        appendBatch(builder, batch, type);
                    }

                    {
                        std::lock_guard<std::mutex> builders_lock(builders_mutex_);
                        std::lock_guard<std::mutex> fields_lock(fields_mutex_);
                        fields[col] = arrow::field(columnName, type);
                        columnTypes[col] = type;
                        builders[col] = builder;
                    }
                }
            };

            // 使用线程池处理任务
            size_t cols_per_thread = maxCol / num_threads;
            for (size_t i = 0; i < num_threads; ++i) {
                size_t start_col = i * cols_per_thread;
                size_t end_col = (i == num_threads - 1) ? maxCol : (i + 1) * cols_per_thread;
                p.push([&, start_col, end_col](int) {
                    processColumn(start_col, end_col);
                });
            }
            p.stop(true);  // 等待所有任务完成
        } catch (const std::exception& e) {
            // 确保所有线程都被正确清理
            for (auto& thread : threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
            throw std::runtime_error("Thread processing failed: " + std::string(e.what()));
        }

        // 构建最终的 Arrow Table
        std::vector<std::shared_ptr<arrow::Array>> arrays;
        arrays.reserve(maxCol);

        for (size_t i = 0; i < maxCol; ++i) {
            std::shared_ptr<arrow::Array> array;
            auto status = builders[i]->Finish(&array);
            if (!status.ok()) throw std::runtime_error(status.message());
            arrays.push_back(array);
        }

        auto schema = arrow::schema(fields);
        return DataFrame(arrow::Table::Make(schema, arrays));

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
*/
