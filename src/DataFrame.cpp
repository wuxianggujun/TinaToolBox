#include "DataFrame.hpp"
#include <arrow/io/file.h>
#include <arrow/csv/api.h>
#include <arrow/table.h>
#include <arrow/type.h>
#include <arrow/compute/api_scalar.h>
#include <arrow/compute/exec.h>
#include <arrow/builder.h>
#include <memory>
#include <xlnt/xlnt.hpp>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <future>
#include <filesystem>
#include <ctime>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <cmath>
#include <mutex>

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <time.h>
#endif

namespace
{
    // 辅助函数：将xlnt datetime转换为时间戳
    int64_t datetime_to_timestamp(const xlnt::datetime& dt) {
        std::tm tm = {};
        tm.tm_year = dt.year - 1900;
        tm.tm_mon = dt.month - 1;
        tm.tm_mday = dt.day;
        tm.tm_hour = dt.hour;
        tm.tm_min = dt.minute;
        tm.tm_sec = dt.second;
        time_t t = ::mktime(&tm);
        if (t == -1) {
            throw std::runtime_error("Failed to convert datetime to timestamp");
        }
        return static_cast<int64_t>(t) * 1000000LL;
    }

    // 辅助函数：将时间戳转换为xlnt datetime
    xlnt::datetime timestamp_to_datetime(int64_t timestamp) {
        time_t unix_time = timestamp / 1000000;
        std::tm* tm = ::localtime(&unix_time);
        if (!tm) {
            throw std::runtime_error("Failed to convert timestamp to datetime");
        }

        return xlnt::datetime(
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec
        );
    }
}

namespace TinaToolBox {
    
    DataFrame::DataFrame(std::shared_ptr<arrow::Table> table) : table_(table) {}

    // 辅助函数：创建一个新的ArrayBuilder
    static std::shared_ptr<arrow::ArrayBuilder> createBuilder(const std::shared_ptr<arrow::DataType>& type) {
        if (!type) {
            throw std::runtime_error("Null type passed to createBuilder");
        }

        arrow::MemoryPool* pool = arrow::default_memory_pool();
        
        switch (type->id()) {
            case arrow::Type::TIMESTAMP: {
                auto timestamp_type = std::static_pointer_cast<arrow::TimestampType>(type);
                return std::make_shared<arrow::TimestampBuilder>(timestamp_type, pool);
            }
            case arrow::Type::INT64: {
                return std::make_shared<arrow::Int64Builder>(pool);
            }
            case arrow::Type::DOUBLE: {
                return std::make_shared<arrow::DoubleBuilder>(pool);
            }
            case arrow::Type::BOOL: {
                return std::make_shared<arrow::BooleanBuilder>(pool);
            }
            default: {
                return std::make_shared<arrow::StringBuilder>(pool);
            }
        }
    }

    DataFrame DataFrame::fromExcel(const std::string &filePath) {
        auto& pool = getThreadPool();
        xlnt::workbook wb;
        
        try {
            wb.load(filePath);
        } catch (const xlnt::exception& e) {
            spdlog::error("Failed to load Excel file: {}", e.what());
            throw std::runtime_error("Failed to load Excel file: " + std::string(e.what()));
        }

        // 获取工作表的副本而不是引用
        xlnt::worksheet ws = wb.active_sheet();
        const auto max_row = ws.highest_row();
        const auto max_column = ws.highest_column().index;

        if (max_row < 1 || max_column < 1) {
            throw std::runtime_error("Excel file is empty");
        }

        spdlog::info("Reading Excel file with {} rows and {} columns", max_row, max_column);
        
        // 减少采样大小，加快类型检测
        const size_t SAMPLE_SIZE = std::min<size_t>(20, max_row - 1);
        const size_t SAMPLE_INTERVAL = std::max<size_t>(1, (max_row - 1) / SAMPLE_SIZE);
        
        std::vector<std::shared_ptr<arrow::DataType>> column_types(max_column);
        std::vector<std::string> column_names(max_column);
        
        // 增加chunk大小以减少内存分配次数
        const size_t estimated_rows = max_row - 1;
        const size_t CHUNK_SIZE = 5000;  // 更大的chunk大小
        const size_t num_chunks = (estimated_rows + CHUNK_SIZE - 1) / CHUNK_SIZE;

        // 预缓存所有列名，减少重复访问
        for (int col = 1; col <= max_column; ++col) {
            column_names[col - 1] = ws.cell(col, 1).to_string();
            if(column_names[col - 1].empty()) {
                column_names[col - 1] = "Column" + std::to_string(col);
            }
        }

        // 使用互斥锁保护worksheet访问
        std::mutex ws_mutex;
        
        // 并行类型检测
        std::vector<std::future<void>> type_futures;
        type_futures.reserve(max_column);
        
        for (int col = 1; col <= max_column; ++col) {
            type_futures.push_back(pool.submit([col, ws, max_row, SAMPLE_SIZE, SAMPLE_INTERVAL, &column_types]() {
                std::unordered_map<arrow::Type::type, int> type_counts;
                bool has_date = false;
                std::vector<xlnt::cell> sample_cells;
                sample_cells.reserve(SAMPLE_SIZE);
                
                // 一次性获取所有样本单元格
                for (size_t sample_idx = 0; sample_idx < SAMPLE_SIZE; ++sample_idx) {
                    size_t row = 2 + sample_idx * SAMPLE_INTERVAL;
                    if (row > max_row) break;
                    sample_cells.push_back(ws.cell(col, row));
                }
                
                // 处理样本数据
                size_t non_empty_count = 0;
                for (const auto& cell : sample_cells) {
                    if (non_empty_count >= 10) break;
                    
                    if (cell.has_value()) {
                        non_empty_count++;
                        if (cell.is_date()) {
                            has_date = true;
                            break;
                        }
                        switch (cell.data_type()) {
                            case xlnt::cell_type::number: {
                                double value = cell.value<double>();
                                double intpart;
                                if (std::modf(value, &intpart) == 0.0) {
                                    type_counts[arrow::Type::INT64]++;
                                } else {
                                    type_counts[arrow::Type::DOUBLE]++;
                                }
                                break;
                            }
                            case xlnt::cell_type::boolean:
                                type_counts[arrow::Type::BOOL]++;
                                break;
                            default:
                                type_counts[arrow::Type::STRING]++;
                                break;
                        }
                    }
                }
                
                if (has_date) {
                    column_types[col - 1] = std::make_shared<arrow::TimestampType>(arrow::TimeUnit::MICRO);
                } else if (!type_counts.empty()) {
                    auto max_type = std::max_element(
                        type_counts.begin(), type_counts.end(),
                        [](const auto& p1, const auto& p2) { return p1.second < p2.second; }
                    );
                    
                    switch (max_type->first) {
                        case arrow::Type::INT64:
                            column_types[col - 1] = std::make_shared<arrow::Int64Type>();
                            break;
                        case arrow::Type::DOUBLE:
                            column_types[col - 1] = std::make_shared<arrow::DoubleType>();
                            break;
                        case arrow::Type::BOOL:
                            column_types[col - 1] = std::make_shared<arrow::BooleanType>();
                            break;
                        default:
                            column_types[col - 1] = std::make_shared<arrow::StringType>();
                            break;
                    }
                } else {
                    column_types[col - 1] = std::make_shared<arrow::StringType>();
                }
            }));
        }

        // 等待所有类型检测完成
        for (auto& fut : type_futures) {
            fut.wait();
        }

        // 创建schema
        std::vector<std::shared_ptr<arrow::Field>> fields;
        fields.reserve(max_column);
        for (size_t i = 0; i < max_column; ++i) {
            fields.push_back(std::make_shared<arrow::Field>(column_names[i], column_types[i]));
        }
        auto schema = std::make_shared<arrow::Schema>(fields);

        // 并行处理所有列
        std::vector<std::shared_ptr<arrow::ChunkedArray>> columns(max_column);
        std::vector<std::future<void>> column_futures;
        column_futures.reserve(max_column);
        
        for (int col = 1; col <= max_column; ++col) {
            column_futures.push_back(pool.submit([col, ws, max_row, CHUNK_SIZE, num_chunks, &column_types, &columns]() {
                std::vector<std::shared_ptr<arrow::Array>> chunks;
                chunks.reserve(num_chunks);
                auto builder = createBuilder(column_types[col-1]);
                
                for (size_t chunk = 0; chunk < num_chunks; ++chunk) {
                    size_t start_row = 2 + chunk * CHUNK_SIZE;
                    size_t end_row = std::min<size_t>(start_row + CHUNK_SIZE, max_row + 1);
                    
                    builder->Reserve(end_row - start_row);
                    
                    // 一次性读取整个chunk的数据
                    std::vector<xlnt::cell> chunk_cells;
                    chunk_cells.reserve(end_row - start_row);
                    for (size_t row = start_row; row < end_row; ++row) {
                        chunk_cells.push_back(ws.cell(col, row));
                    }
                    
                    // 处理chunk数据
                    for (const auto& cell : chunk_cells) {
                        arrow::Status status;

                        if (!cell.has_value()) {
                            status = builder->AppendNull();
                        } else {
                            switch (column_types[col-1]->id()) {
                                case arrow::Type::TIMESTAMP: {
                                    auto timestampBuilder = static_cast<arrow::TimestampBuilder*>(builder.get());
                                    if (cell.is_date()) {
                                        auto dt = cell.value<xlnt::datetime>();
                                        status = timestampBuilder->Append(datetime_to_timestamp(dt));
                                    } else {
                                        status = builder->AppendNull();
                                    }
                                    break;
                                }
                                case arrow::Type::INT64: {
                                    auto intBuilder = static_cast<arrow::Int64Builder*>(builder.get());
                                    if (cell.data_type() == xlnt::cell_type::number) {
                                        status = intBuilder->Append(static_cast<int64_t>(cell.value<double>()));
                                    } else {
                                        status = builder->AppendNull();
                                    }
                                    break;
                                }
                                case arrow::Type::DOUBLE: {
                                    auto doubleBuilder = static_cast<arrow::DoubleBuilder*>(builder.get());
                                    if (cell.data_type() == xlnt::cell_type::number) {
                                        status = doubleBuilder->Append(cell.value<double>());
                                    } else {
                                        status = builder->AppendNull();
                                    }
                                    break;
                                }
                                case arrow::Type::BOOL: {
                                    auto boolBuilder = static_cast<arrow::BooleanBuilder*>(builder.get());
                                    if (cell.data_type() == xlnt::cell_type::boolean) {
                                        status = boolBuilder->Append(cell.value<bool>());
                                    } else {
                                        status = builder->AppendNull();
                                    }
                                    break;
                                }
                                default: {
                                    auto stringBuilder = static_cast<arrow::StringBuilder*>(builder.get());
                                    status = stringBuilder->Append(cell.to_string());
                                    break;
                                }
                            }
                        }

                        if (!status.ok()) {
                            throw std::runtime_error("Failed to append value: " + status.ToString());
                        }
                    }

                    std::shared_ptr<arrow::Array> chunk_array;
                    auto status = builder->Finish(&chunk_array);
                    if (!status.ok()) {
                        throw std::runtime_error("Failed to finalize array: " + status.ToString());
                    }
                    chunks.push_back(chunk_array);
                    
                    builder = createBuilder(column_types[col-1]);
                }
                
                columns[col-1] = std::make_shared<arrow::ChunkedArray>(chunks);
            }));
        }

        // 等待所有列处理完成
        for (auto& fut : column_futures) {
            fut.wait();
        }

        return DataFrame(arrow::Table::Make(schema, columns));
    }

    std::vector<std::string> DataFrame::getColumnNames() const {
        std::vector<std::string> names;
        if (table_) {
            for (const auto &field : table_->schema()->fields()) {
                names.push_back(field->name());
            }
        }
        return names;
    }

    std::shared_ptr<arrow::ChunkedArray> DataFrame::getColumn(const std::string &name) const {
        if (!table_) {
            return nullptr;
        }
        int i = table_->schema()->GetFieldIndex(name);
        if(i == -1)
            return nullptr;
        return table_->column(i);
    }

    arrow::Result<std::shared_ptr<arrow::RecordBatch>> DataFrame::getRow(int64_t index) const {
        if (!table_ || index < 0 || index >= table_->num_rows()) {
            return arrow::Status::Invalid("Invalid row index");
        }
        
        // Create a slice of the table for the specific row
        auto sliced_table = table_->Slice(index, 1);
        std::shared_ptr<arrow::RecordBatch> batch;
        ARROW_RETURN_NOT_OK(arrow::TableBatchReader(*sliced_table).ReadNext(&batch));
        return batch;
    }

    arrow::Result<DataFrame> DataFrame::filter(
        const std::string& column,
        const std::shared_ptr<arrow::Scalar>& value,
        const std::string& comparison_operator
    ) const {
        if (!table_) {
            return arrow::Status::Invalid("DataFrame is empty");
        }

        auto col = getColumn(column);
        if (!col) {
            return arrow::Status::Invalid("Column not found");
        }

        // Create comparison kernel
        std::string kernel_name;
        if (comparison_operator == "equal") {
            kernel_name = "equal";
        } else if (comparison_operator == "not_equal") {
            kernel_name = "not_equal";
        } else if (comparison_operator == "greater") {
            kernel_name = "greater";
        } else if (comparison_operator == "greater_equal") {
            kernel_name = "greater_equal";
        } else if (comparison_operator == "less") {
            kernel_name = "less";
        } else if (comparison_operator == "less_equal") {
            kernel_name = "less_equal";
        } else {
            return arrow::Status::Invalid("Invalid comparison operator");
        }

        // Perform comparison
        arrow::Datum col_datum(col);
        arrow::Datum value_datum(value);
        ARROW_ASSIGN_OR_RAISE(auto mask_datum, 
            arrow::compute::CallFunction(kernel_name, {col_datum, value_datum}));

        // Perform filtering
        arrow::Datum table_datum(table_);
        ARROW_ASSIGN_OR_RAISE(auto filtered_datum,
            arrow::compute::CallFunction("filter", {table_datum, mask_datum}));

        return DataFrame(filtered_datum.table());
    }

    arrow::Result<DataFrame> DataFrame::sort(
        const std::string& column, 
        bool ascending
    ) const {
        if (!table_) {
            return arrow::Status::Invalid("DataFrame is empty");
        }

        auto col = getColumn(column);
        if (!col) {
            return arrow::Status::Invalid("Column not found");
        }
        arrow::compute::SortOptions options({arrow::compute::SortKey(column, ascending ? arrow::compute::SortOrder::Ascending : arrow::compute::SortOrder::Descending)});

        ARROW_ASSIGN_OR_RAISE(auto indices, arrow::compute::SortIndices(table_, options));
        ARROW_ASSIGN_OR_RAISE(auto sorted_table, arrow::compute::Take(table_, indices));

        return DataFrame(sorted_table.table());
    }

    template<typename T>
    arrow::Result<T> DataFrame::getValue(int64_t row, const std::string &column) const {
        if (!table_) {
            return arrow::Status::Invalid("DataFrame is empty");
        }
        if (row < 0 || row >= table_->num_rows()) {
            return arrow::Status::Invalid("Row index out of bounds");
        }
        auto col = getColumn(column);
        if (!col) {
            return arrow::Status::Invalid("Column not found");
        }
        if (col->num_chunks() == 0) {
            return arrow::Status::Invalid("Column has no data");
        }
        
        if constexpr (std::is_same_v<T, std::string>) {
            if (col->type()->id() != arrow::Type::STRING && col->type()->id() != arrow::Type::LARGE_STRING) {
                return arrow::Status::TypeError("Column type is not string");
            }
            auto array = std::static_pointer_cast<arrow::StringArray>(col->chunk(0));
            return array->GetString(row);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            if (col->type()->id() != arrow::Type::INT64) {
                return arrow::Status::TypeError("Column type is not int64");
            }
            auto array = std::static_pointer_cast<arrow::Int64Array>(col->chunk(0));
            return array->Value(row);
        } else if constexpr (std::is_same_v<T, double>) {
            if (col->type()->id() != arrow::Type::DOUBLE) {
                return arrow::Status::TypeError("Column type is not double");
            }
            auto array = std::static_pointer_cast<arrow::DoubleArray>(col->chunk(0));
            return array->Value(row);
        } else if constexpr (std::is_same_v<T, bool>) {
            if (col->type()->id() != arrow::Type::BOOL) {
                return arrow::Status::TypeError("Column type is not bool");
            }
            auto array = std::static_pointer_cast<arrow::BooleanArray>(col->chunk(0));
            return array->Value(row);
        } else {
            return arrow::Status::NotImplemented("Type not supported");
        }
    }

    bool DataFrame::toSaveExcel(const std::string &filePath, bool forceOverwrite) const {
        if (!forceOverwrite && std::filesystem::exists(filePath)) {
            spdlog::error("File already exists: {}", filePath);
            return false;
        }

        if (!table_) {
            spdlog::error("DataFrame is empty, nothing to save.");
            return false;
        }

        xlnt::workbook wb;
        xlnt::worksheet ws = wb.active_sheet();

        // Write header
        int col_index = 1;
        for (const auto& field : table_->schema()->fields()) {
            ws.cell(col_index, 1).value(field->name());
            col_index++;
        }

        // Write data
        for (int64_t row_index = 0; row_index < table_->num_rows(); ++row_index) {
            for (int col_index = 0; col_index < table_->num_columns(); ++col_index) {
                auto cell = ws.cell(col_index + 1, row_index + 2);
                auto column = table_->column(col_index);

                if (column->chunk(0)->IsNull(row_index)) {
                    continue; // Skip null values
                }

                switch (column->type()->id()) {
                    case arrow::Type::INT64: {
                        auto int_array = std::static_pointer_cast<arrow::Int64Array>(column->chunk(0));
                        cell.value(static_cast<int64_t>(int_array->Value(row_index)));
                        break;
                    }
                    case arrow::Type::DOUBLE: {
                        auto double_array = std::static_pointer_cast<arrow::DoubleArray>(column->chunk(0));
                        cell.value(double_array->Value(row_index));
                        break;
                    }
                    case arrow::Type::STRING: {
                        auto string_array = std::static_pointer_cast<arrow::StringArray>(column->chunk(0));
                        cell.value(string_array->GetString(row_index));
                        break;
                    }
                    case arrow::Type::BOOL: {
                        auto bool_array = std::static_pointer_cast<arrow::BooleanArray>(column->chunk(0));
                        cell.value(bool_array->Value(row_index));
                        break;
                    }
                    case arrow::Type::TIMESTAMP: {
                        auto timestamp_array = std::static_pointer_cast<arrow::TimestampArray>(column->chunk(0));
                        auto timestamp = timestamp_array->Value(row_index);
                        xlnt::datetime dt = timestamp_to_datetime(timestamp);
                        cell.value(dt);
                        cell.number_format(xlnt::number_format("yyyy-mm-dd"));
                        break;
                    }
                    default:
                        spdlog::warn("Unsupported column type for Excel export: {}", column->type()->ToString());
                        break;
                }
            }
        }

        try {
            wb.save(filePath);
            spdlog::info("DataFrame successfully saved to: {}", filePath);
            return true;
        } catch (const xlnt::exception& e) {
            spdlog::error("Failed to save Excel file: {}", e.what());
            return false;
        }
    }

    void DataFrame::appendBatch(std::shared_ptr<arrow::ArrayBuilder>& builder,
                               const std::vector<std::variant<std::string, double, int64_t>>& batch,
                               const std::shared_ptr<arrow::DataType>& type) {
        
    }

    template arrow::Result<std::string> DataFrame::getValue<std::string>(int64_t row, const std::string &column) const;
    template arrow::Result<int64_t> DataFrame::getValue<int64_t>(int64_t row, const std::string &column) const;
    template arrow::Result<double> DataFrame::getValue<double>(int64_t row, const std::string &column) const;
    template arrow::Result<bool> DataFrame::getValue<bool>(int64_t row, const std::string &column) const;
}