#include "DataFrame.hpp"
#include <OpenXLSX.hpp>
#include <utf8.h>

namespace TinaToolBox {
    DataFrame DataFrame::fromExcel(const std::string &filePath) {
        DataFrame df;
        OpenXLSX::XLDocument doc;
        doc.open(filePath);

        // 获取第一个工作表
        auto wbk = doc.workbook();
        if (wbk.sheetCount() == 0) {
            throw std::runtime_error("Excel file contains no sheets");
        }

        // 使用第一个sheet   
        auto wks = wbk.sheet(1).get<OpenXLSX::XLWorksheet>();
        df.columnNames_.reserve(wks.columnCount());
        df.columns_.reserve(wks.columnCount());

        // 获取第一行作为表头
        size_t colCount = 0;
        while (true) {
            colCount++; // 先增加计数
            auto cell = wks.cell(1, colCount);
            if (cell.value().type() == OpenXLSX::XLValueType::Empty) {
                colCount--; // 如果是空单元格，回退计数
                break;
            }
            // 直接获取字符串，不进行UTF-8转换
            std::string header = cell.value().get<std::string>();
            df.columnNames_.push_back(header);
            df.columns_.emplace_back().reserve(wks.rowCount());
        }
        // colCount--; // 调整为实际列数
        // if (colCount == 0) {
        //     throw std::runtime_error("No columns found in Excel file");
        // }
        // 读取数据
        size_t rowNum = 2; // 从第二行开始读取数据
        while (true) {
            auto firstCell = wks.cell(rowNum, 1);

            // 检查第一列的单元格是否为空
            if (firstCell.value().type() == OpenXLSX::XLValueType::Empty) {
                // 再检查整行是否都为空
                bool isEmptyRow = true;
                for (size_t col = 1; col <= colCount; col++) {
                    auto cell = wks.cell(rowNum, col);
                    if (cell.value().type() != OpenXLSX::XLValueType::Empty) {
                        isEmptyRow = false;
                        break;
                    }
                }
                if (isEmptyRow) break;
            }
            // std::vector<DataValue> rowData;
            // rowData.reserve(colCount); // 预分配空间

            for (size_t col = 1; col <= colCount; col++) {
                auto cell = wks.cell(rowNum, col);

                switch (cell.value().type()) {
                    case OpenXLSX::XLValueType::Empty:
                        df.columns_[col - 1].emplace_back(std::string(""));
                        break;

                    case OpenXLSX::XLValueType::Integer:
                        df.columns_[col - 1].emplace_back(static_cast<int>(cell.value().get<int64_t>()));
                        break;

                    case OpenXLSX::XLValueType::Float:
                        df.columns_[col - 1].emplace_back(cell.value().get<double>());
                        break;

                    default:
                        df.columns_[col - 1].emplace_back(cell.value().get<std::string>());
                        break;
                }
            }
            df.rowCount_++;
            rowNum++;
        }
        doc.close();
        return df;
    }

    void DataFrame::addColumn(const std::string &name, const Column &data) {
        if (rowCount_ == 0) {
            rowCount_ = data.size();
        }
        columnNames_.push_back(name);
        columns_.push_back(data);
    }

    void DataFrame::addRow(const std::vector<DataValue> &row) {
        if (row.size() != columns_.size()) {
            throw std::runtime_error("Row size (" + std::to_string(row.size()) +
                                     ") does not match column size (" +
                                     std::to_string(columns_.size()) + ")");
        }
        for (size_t i = 0; i < row.size(); ++i) {
            columns_[i].push_back(row[i]);
        }
        rowCount_++;
    }

    void DataFrame::removeColumn(const std::string &name) {
        auto it = std::find(columnNames_.begin(), columnNames_.end(), name);
        if (it == columnNames_.end()) {
            throw std::runtime_error("Column not found: " + name);
        }
        size_t index = std::distance(columnNames_.begin(), it);
        columnNames_.erase(it);
        columns_.erase(columns_.begin() + index);
    }

    void DataFrame::removeRow(const size_t &index) {
        if (index >= rowCount_) {
            throw std::out_of_range("Row index out of range");
        }
        for (auto &column: columns_) {
            column.erase(column.begin() + index);
        }
        rowCount_--;
    }

    const Column DataFrame::getColumn(const std::string &name) const {
        auto it = std::find(columnNames_.begin(), columnNames_.end(), name);
        if (it == columnNames_.end()) {
            throw std::runtime_error("Column not found: " + name);
        }
        size_t index = std::distance(columnNames_.begin(), it);
        return columns_[index];
    }

    std::vector<DataValue> DataFrame::getRow(const size_t &index) const {
        if (index >= rowCount_) {
            throw std::out_of_range("Row index out of range");
        }
        std::vector<DataValue> row;
        row.reserve(columns_.size());
        for (const auto &column: columns_) {
            row.push_back(column[index]);
        }
        return row;
    }

    DataValue DataFrame::getValue(const size_t &row, const std::string &column) const {
        return getColumn(column)[row];
    }

    size_t DataFrame::rowCount() const {
        return rowCount_;
    }

    size_t DataFrame::columnCount() const {
        return columns_.size();
    }

    std::vector<std::string> DataFrame::getColumnNames() const {
        return columnNames_;
    }

    DataFrame DataFrame::filter(const std::string &column, const DataValue &value) {
        DataFrame result;
        auto colData = getColumn(column);
        for (size_t i = 0; i < rowCount_; i++) {
            if (colData[i] == value) {
                result.addRow(getRow(i));
            }
        }
        return result;
    }

    DataFrame DataFrame::sort(const std::string &column, bool ascending) {
        DataFrame result = *this;
        auto colData = getColumn(column);
        std::vector<size_t> indices(rowCount_);
        for (size_t i = 0; i < rowCount_; i++) {
            indices[i] = i;
        }

        std::sort(indices.begin(), indices.end(),
                  [&colData, ascending](size_t i1, size_t i2) {
                      return ascending ? colData[i1] < colData[i2] : colData[i1] > colData[i2];
                  });

        // 重新排序所有列
        for (auto &col: result.columns_) {
            Column newColumn;
            newColumn.reserve(rowCount_);
            for (const size_t idx: indices) {
                newColumn.push_back(col[idx]);
            }
            col = std::move(newColumn);
        }

        return result;
    }

    bool DataFrame::toSaveExcel(const std::string &filePath, bool forceOverwrite) const {
        try {
            OpenXLSX::XLDocument doc;
            doc.create(filePath,forceOverwrite);
            const auto wks = doc.workbook().worksheet("Sheet1");

            // 写入表头
            for (size_t col = 0; col < columnNames_.size(); ++col) {
                wks.cell(1, col + 1).value() = columnNames_[col];
            }

            // 写入数据
            for (size_t row = 0; row < rowCount_; ++row) {
                for (size_t col = 0; col < columns_.size(); ++col) {
                    OpenXLSX::XLCellAssignable cell = wks.cell(row + 2, col + 1);
                    std::visit([&cell](const auto &value) {
                        if constexpr (std::is_same_v<std::decay_t<decltype(value)>, double>) {
                            // 处理浮点数
                            cell.value() = std::to_string(value);
                        } else {
                            cell.value() = value;
                        }
                    }, columns_[col][row]);
                }
            }
            doc.save();
            doc.close();
            return true;
        } catch (...) {
            return false;
        }
    }
}
