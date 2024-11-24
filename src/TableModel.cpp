//
// Created by wuxianggujun on 2024/11/24.
//

#include "TableModel.hpp"


TableModel::TableModel(QObject* parent) : QAbstractTableModel(parent) {
}

int TableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return data_.size();
}

int TableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return data_.isEmpty() ? 0 : data_[0].size();
}

QVariant TableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        int row = index.row();
        int col = index.column();

        // 检查是否在合并单元格范围内
        for (const auto& cell : mergedCells_) {
            const auto& startPos = cell.first;
            const auto& endPos = cell.second;
            
            int startRow = startPos.first;
            int startCol = startPos.second;
            int endRow = endPos.first;
            int endCol = endPos.second;

            if (startRow <= row && row <= endRow && 
                startCol <= col && col <= endCol) {
                // 如果是合并单元格的起始位置，显示数据
                if (row == startRow && col == startCol) {
                    return data_[row][col];
                }
                // 如果是合并单元格内的其他位置，不显示数据
                return QVariant();
            }
        }

        // 不在任何合并单元格范围内，正常显示数据
        return data_[row][col];
    }
    
    return QVariant();
}

QString TableModel::getExcelColumnName(int columnNumber) const {
    QString result;
    while (columnNumber >= 0) {
        result.prepend(QChar('A' + (columnNumber % 26)));
        columnNumber = columnNumber / 26 - 1;
    }
    return result;
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            // 使用Excel风格的列名（A, B, C, ...）
            return getExcelColumnName(section);
        } else {
            return QString::number(section + 1);
        }
    }
    return QVariant();
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool TableModel::setData(
    const QVector<QVector<QVariant>>& data,
    const QVector<QPair<QPair<int, int>, QPair<int, int>>>& mergedCells) {
    beginResetModel();
    data_ = data;
    mergedCells_ = mergedCells;
    endResetModel();
    return true;
}