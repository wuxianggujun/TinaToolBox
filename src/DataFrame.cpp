#include "DataFrame.hpp"
#include <arrow/io/file.h>
#include <arrow/csv/api.h>
#include <arrow/table.h>
#include <arrow/type.h>
#include <arrow/compute/api_scalar.h>
#include <arrow/compute/exec.h>
#include <memory>
#include <xlnt/xlnt.hpp>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <future>
#include <filesystem>
#include <ctime>

namespace TinaToolBox {

    DataFrame::DataFrame(std::shared_ptr<arrow::Table> table) : table_(table) {}

    DataFrame DataFrame::fromExcel(const std::string &filePath) {
        auto& pool = getThreadPool();
        xlnt::workbook wb;
        try {
            wb.load(filePath);
        } catch (const xlnt::exception& e) {
            spdlog::error("Failed to load Excel file: {}", e.what());
            throw std::runtime_error("Failed to load Excel file: " + std::string(e.what()));
        }

        xlnt::worksheet ws = wb.active_sheet();
        auto max_row = ws.highest_row();
        auto max_column = ws.highest_column().index;

        std::vector<std::shared_ptr<arrow::Field>> fields;
        std::vector<std::shared_ptr<arrow::ArrayBuilder>> builders;

        // Determine column types and create builders in parallel
        std::vector<std::future<std::pair<std::shared_ptr<arrow::Field>, std::shared_ptr<arrow::ArrayBuilder>>>> futures;
        for (int col = 1; col <= max_column; ++col) {
            futures.push_back(pool.submit([&ws, col]() -> std::pair<std::shared_ptr<arrow::Field>, std::shared_ptr<arrow::ArrayBuilder>> {
                std::shared_ptr<arrow::DataType> type;
                // Inspect first few rows to determine type
                for (int row = 2; row <= std::min(11, (int)ws.highest_row()); ++row) {
                    auto cell = ws.cell(col, row);
                    if (cell.has_value()) {
                        if (cell.is_date()) {
                             type = std::make_shared<arrow::TimestampType>(arrow::TimeUnit::MICRO);
                             break;
                        }
                        switch (cell.data_type()) {
                            case xlnt::cell_type::number: {
                                double value = cell.value<double>();
                                double intpart;
                                if (std::modf(value, &intpart) == 0.0) {
                                    type = std::make_shared<arrow::Int64Type>();
                                } else {
                                    type = std::make_shared<arrow::DoubleType>();
                                }
                                break;
                            }
                            case xlnt::cell_type::boolean:
                                type = std::make_shared<arrow::BooleanType>();
                                break;
                            case xlnt::cell_type::inline_string:
                            case xlnt::cell_type::shared_string:
                            default:
                                type = std::make_shared<arrow::StringType>();
                                break;
                        }
                        break;
                    }
                }

                if (!type) {
                    type = std::make_shared<arrow::StringType>();
                }

                std::shared_ptr<arrow::ArrayBuilder> builder;
                
                if (type->id() == arrow::Type::TIMESTAMP) {
                    builder = std::make_shared<arrow::TimestampBuilder>(type, arrow::default_memory_pool());
                } else if (type->id() == arrow::Type::INT64) {
                    builder = std::make_shared<arrow::Int64Builder>();
                } else if (type->id() == arrow::Type::DOUBLE) {
                    builder = std::make_shared<arrow::DoubleBuilder>();
                } else if (type->id() == arrow::Type::BOOL) {
                    builder = std::make_shared<arrow::BooleanBuilder>();
                } else {
                    builder = std::make_shared<arrow::StringBuilder>();
                }

                std::string columnName = ws.cell(col, 1).to_string();
                if(columnName.empty()){
                    columnName = "Column" + std::to_string(col);
                }
                return {std::make_shared<arrow::Field>(columnName, type), builder};
            }));
        }

        for (auto& fut : futures) {
            auto [field, builder] = fut.get();
            fields.push_back(field);
            builders.push_back(builder);
        }

        auto schema = std::make_shared<arrow::Schema>(fields);

        // Process rows in parallel
        std::vector<std::future<arrow::Status>> append_futures;
        std::size_t n_threads = std::thread::hardware_concurrency();
        std::size_t rows_per_thread = (max_row - 1 + n_threads - 1) / n_threads; 

        for (std::size_t i = 0; i < n_threads; ++i) {
            append_futures.push_back(pool.submit([&, i]() -> arrow::Status {
                std::size_t start_row = 2 + i * rows_per_thread;
                std::size_t end_row = std::min(start_row + rows_per_thread, (std::size_t)max_row + 1);

                for (std::size_t row = start_row; row < end_row; ++row) {
                    for (int col = 1; col <= max_column; ++col) {
                        auto cell = ws.cell(col, row);
                        auto& builder = builders[col - 1];

                        if (!cell.has_value()) {
                            ARROW_RETURN_NOT_OK(builder->AppendNull());
                            continue;
                        }
                        
                        if (fields[col-1]->type()->id() == arrow::Type::TIMESTAMP) {
                            auto timestampBuilder = dynamic_cast<arrow::TimestampBuilder*>(builder.get());
                            if (cell.is_date()) {
                                auto dt = cell.value<xlnt::datetime>();
                                // Convert xlnt datetime to timestamp
                                std::tm tm = {};
                                tm.tm_year = dt.year - 1900;
                                tm.tm_mon = dt.month - 1;
                                tm.tm_mday = dt.day;
                                tm.tm_hour = dt.hour;
                                tm.tm_min = dt.minute;
                                tm.tm_sec = dt.second;
                                int64_t timestamp = ::mktime(&tm) * 1000000;
                                ARROW_RETURN_NOT_OK(timestampBuilder->Append(timestamp));
                            } else {
                                ARROW_RETURN_NOT_OK(builder->AppendNull());
                            }
                        } else if (fields[col - 1]->type()->id() == arrow::Type::INT64) {
                            auto intBuilder = dynamic_cast<arrow::Int64Builder*>(builder.get());
                            if (cell.data_type() == xlnt::cell_type::number) {
                                ARROW_RETURN_NOT_OK(intBuilder->Append(static_cast<int64_t>(cell.value<double>())));
                            } else {
                                ARROW_RETURN_NOT_OK(builder->AppendNull());
                            }
                        } else if (fields[col - 1]->type()->id() == arrow::Type::DOUBLE) {
                            auto doubleBuilder = dynamic_cast<arrow::DoubleBuilder*>(builder.get());
                            if (cell.data_type() == xlnt::cell_type::number) {
                                ARROW_RETURN_NOT_OK(doubleBuilder->Append(cell.value<double>()));
                            } else {
                                ARROW_RETURN_NOT_OK(builder->AppendNull());
                            }
                        } else if (fields[col - 1]->type()->id() == arrow::Type::BOOL) {
                            auto boolBuilder = dynamic_cast<arrow::BooleanBuilder*>(builder.get());
                            if (cell.data_type() == xlnt::cell_type::boolean) {
                                ARROW_RETURN_NOT_OK(boolBuilder->Append(cell.value<bool>()));
                            } else {
                                ARROW_RETURN_NOT_OK(builder->AppendNull());
                            }
                        } else {
                            auto stringBuilder = dynamic_cast<arrow::StringBuilder*>(builder.get());
                            ARROW_RETURN_NOT_OK(stringBuilder->Append(cell.to_string()));
                        }
                    }
                }
                return arrow::Status::OK();
            }));
        }

        // Wait for all append tasks to complete and check their status
        for (auto& fut : append_futures) {
            auto status = fut.get();
            if (!status.ok()) {
                throw std::runtime_error("Failed to process data: " + status.ToString());
            }
        }

        std::vector<std::shared_ptr<arrow::Array>> arrays;
        for (auto& builder : builders) {
            std::shared_ptr<arrow::Array> array;
            arrow::Status status = builder->Finish(&array);
            if (!status.ok()) {
                spdlog::error("Failed to finalize array: {}", status.ToString());
                throw std::runtime_error("Failed to finalize array: " + status.ToString());
            }
            arrays.push_back(array);
        }

        auto table = arrow::Table::Make(schema, arrays);
        return DataFrame(table);
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
                        time_t unix_time = timestamp / 1000000;
                        #ifdef _WIN32
                        struct tm timeinfo;
                        if (localtime_s(&timeinfo, &unix_time) == 0) {
                            xlnt::datetime dt(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, 
                                           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                            cell.value(dt);
                            cell.number_format(xlnt::number_format("yyyy-mm-dd"));
                        }
                        #else
                        struct tm timeinfo;
                        if (localtime_r(&unix_time, &timeinfo) != nullptr) {
                            xlnt::datetime dt(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, 
                                           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                            cell.value(dt);
                            cell.number_format(xlnt::number_format("yyyy-mm-dd"));
                        }
                        #endif
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