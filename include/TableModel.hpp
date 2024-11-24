//
// Created by wuxianggujun on 2024/11/24.
//

#ifndef TINA_TOOL_BOX_TABLE_MODEL_HPP
#define TINA_TOOL_BOX_TABLE_MODEL_HPP

#include <QAbstractTableModel>
#include <QPair>
#include <QVector>
#include <QString>
#include "spdlog/spdlog.h"

class TableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit TableModel(QObject* parent = nullptr);
    
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, 
                       int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    
    bool setData(const QVector<QVector<QVariant>>& data,
                 const QVector<QPair<QPair<int, int>, QPair<int, int>>>& mergedCells = {});

private:
    QString getExcelColumnName(int columnNumber) const;
    
    QVector<QVector<QVariant>> data_;
    QVector<QPair<QPair<int, int>, QPair<int, int>>> mergedCells_;
};



#endif //TINA_TOOL_BOX_TABLE_MODEL_HPP
