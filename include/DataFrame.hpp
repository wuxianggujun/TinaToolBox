#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <algorithm>

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

        Column getColumn(const std::string &name) const;

        std::vector<DataValue> getRow(const size_t &index) const;

        DataValue getValue(const size_t &row, const std::string &column) const;

        size_t rowCount() const;

        size_t columnCount() const;

        std::vector<std::string> getColumnNames() const;

        DataFrame filter(const std::string &column, const DataValue &value);

        DataFrame sort(const std::string &column, bool ascending = true);

        bool toSaveExcel(const std::string &filePath) const;

    private:
        std::vector<std::string> columnNames_;
        std::map<std::string, Column> columns_;
        size_t rowCount_ = 0;
    };
}
