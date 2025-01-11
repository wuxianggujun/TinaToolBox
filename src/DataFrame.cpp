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
        std::vector<std::string> headers;

        // 获取第一行作为表头
        size_t colCount = 0;
        while (true) {
            colCount++; // 先增加计数
            auto cell = wks.cell(1, colCount);
            if (cell.value().type() == OpenXLSX::XLValueType::Empty) {
                colCount--; // 如果是空单元格，回退计数
                break;
            }
            // 使用utfcpp处理UTF-8字符串
            // 直接获取字符串，不进行UTF-8转换
            std::string header = cell.value().get<std::string>();
            df.columnNames_.push_back(header);
            df.columns_[header] = Column();
            df.columns_[header].reserve(wks.rowCount());  // 预估行数，可以根据实际情况调整
        }
        colCount--; // 调整为实际列数
        if (colCount == 0) {
            throw std::runtime_error("No columns found in Excel file");
        }
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
            std::vector<DataValue> rowData;
            rowData.reserve(colCount); // 预分配空间

            for (size_t col = 1; col <= colCount; col++) {
                auto cell = wks.cell(rowNum, col);

                switch (cell.value().type()) {
                    case OpenXLSX::XLValueType::Empty:
                        rowData.emplace_back(std::string(""));
                        break;

                    case OpenXLSX::XLValueType::Integer:
                        rowData.emplace_back(static_cast<int>(cell.value().get<int64_t>()));
                        break;

                    case OpenXLSX::XLValueType::Float:
                        rowData.emplace_back(cell.value().get<double>());
                        break;

                    default:
                        rowData.emplace_back(cell.value().get<std::string>());
                        break;
                }
            }
            df.addRow(rowData);
            rowNum++;
        }

        doc.close();
        return df;
    }

    void DataFrame::addColumn(const std::string &name, const Column &data) {
        if (rowCount_ == 0) {
            rowCount_ = data.size();
        }
        columns_[name] = data;
    }

    void DataFrame::addRow(const std::vector<DataValue> &row) {
        if (row.size() != columns_.size()) {
            throw std::runtime_error("Row size (" + std::to_string(row.size()) +
                                     ") does not match column size (" +
                                     std::to_string(columns_.size()) + ")");
        }

        size_t col = 0;
        for (auto &[_, column]: columns_) {
            column.push_back(row[col++]);
        }
        rowCount_++;
    }

    void DataFrame::removeColumn(const std::string &name) {
        columns_.erase(name);
    }

    void DataFrame::removeRow(const size_t &index) {
        if (index >= rowCount_) {
            throw std::out_of_range("Row index out of range");
        }
        for (auto &[_, column]: columns_) {
            column.erase(column.begin() + index);
        }
        rowCount_--;
    }

    Column DataFrame::getColumn(const std::string &name) const {
        auto it = columns_.find(name);
        if (it == columns_.end()) {
            throw std::runtime_error("Column not found: " + name);
        }
        return it->second;
    }

    std::vector<DataValue> DataFrame::getRow(const size_t &index) const {
        if (index >= rowCount_) {
            throw std::out_of_range("Row index out of range");
        }
        std::vector<DataValue> row;
        for (const auto &[_, column]: columns_) {
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
        for (auto &[_, column]: result.columns_) {
            Column newColumn;
            newColumn.reserve(rowCount_);
            for (size_t idx: indices) {
                newColumn.push_back(column[idx]);
            }
            column = std::move(newColumn);
        }

        return result;
    }

    bool DataFrame::toSaveExcel(const std::string &filepath) const {
        try {
            OpenXLSX::XLDocument doc;
            doc.create(filepath);
            auto wks = doc.workbook().worksheet("Sheet1");

            // 写入表头
            size_t col = 1;
            for (const auto &[name, _]: columns_) {
                wks.cell(1, col++).value() = name;
            }

            // 写入数据
            for (size_t row = 0; row < rowCount_; ++row) {
                col = 1;
                for (const auto &[_, column]: columns_) {
                    OpenXLSX::XLCellAssignable cell = wks.cell(row + 2, col++);
                    std::visit([&cell](const auto &value) {
                        cell.value() = value;
                    }, column[row]);
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
