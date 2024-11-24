//
// Created by wuxianggujun on 2024/11/24.
//

#ifndef TINA_TOOL_BOX_EXCEL_PROCESSOR_HPP
#define TINA_TOOL_BOX_EXCEL_PROCESSOR_HPP

#include <QString>
#include <QVector>
#include <QPair>
#include <QVariant>

struct SheetInfo {
    QString sheetName;
    // 其他sheet相关信息
};

class ExcelProcessor {
public:
    QVector<SheetInfo> readExcelStructure(const QString &filePath);

    std::pair<QVector<QVector<QVariant> >,
        QVector<QPair<QPair<int, int>, QPair<int, int> > > >
    readSheetData(int sheetIndex);
};

#endif //TINA_TOOL_BOX_EXCEL_PROCESSOR_HPP
