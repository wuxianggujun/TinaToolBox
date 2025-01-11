#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <variant>

namespace TinaToolBox {
    using DataValue = std::variant<int, double, std::string>;
    using Column = std::vector<DataValue>;

    class DataFrame {
    public:
        DataFrame() = default;

        static DataFrame fromExcel(const std::string &filePath);

        void addColumn(const std::string &name, const Column &data);

        void addRow(const std::vector<DataValue> &row);

        void removeColumn(const std::string &name);

        void removeRow(const size_t &index);

        [[nodiscard]] const Column getColumn(const std::string &name) const;

        [[nodiscard]] std::vector<DataValue> getRow(const size_t &index) const;

        [[nodiscard]] DataValue getValue(const size_t &row, const std::string &column) const;

        [[nodiscard]] size_t rowCount() const;

        [[nodiscard]] size_t columnCount() const;

        [[nodiscard]] std::vector<std::string> getColumnNames() const;

        DataFrame filter(const std::string &column, const DataValue &value);

        DataFrame sort(const std::string &column, bool ascending = true);

        [[nodiscard]] bool toSaveExcel(const std::string &filePath, bool forceOverwrite) const;

        template<typename T>
        [[nodiscard]] T getValue(const size_t &row, const std::string &column) const {
            auto value = getValue(row, column);
            return std::visit([](const auto &v)-> T {
                using ValueType = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<ValueType, T>) {
                    return v;
                } else if constexpr (std::is_arithmetic_v<T> && std::is_arithmetic_v<ValueType>) {
                    return static_cast<T>(v);
                } else if constexpr (std::is_same_v<ValueType, std::string>) {
                    return std::to_string(v);
                } else {
                    throw std::runtime_error("Unsupported type conversion");
                }
            }, value);
        }

    private:
        std::vector<std::string> columnNames_;
        std::vector<Column> columns_;
        size_t rowCount_ = 0;
    };
}
