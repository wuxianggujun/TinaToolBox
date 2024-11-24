//
// Created by wuxianggujun on 2024/11/24.
//

#ifndef TINA_TOOL_BOX_MERGED_TABLEVIEW_HPP
#define TINA_TOOL_BOX_MERGED_TABLEVIEW_HPP


#include <QTableView>
#include <QMap>
#include <QPair>

class MergedTableView : public QTableView{
Q_OBJECT
public:
    explicit MergedTableView(QWidget* parent = nullptr);
    void setMergedCells(const QVector<QPair<QPair<int,int>,QPair<int,int>>> & mergedCells);

private:
    void setupUI();
};



#endif //TINA_TOOL_BOX_MERGED_TABLEVIEW_HPP
