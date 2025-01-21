#pragma once

#include <QObject>
#include "Document.hpp"
#include "DocumentView.hpp"
#include "MergedTableView.hpp"
#include "TableModel.hpp"
// #include <xlsxdocument.h>
// #include <xlsxcellrange.h>
#include <memory>

namespace TinaToolBox {
    class ExcelDocumentView :public QObject,public IDocumentView {
        Q_OBJECT
    public:
        explicit  ExcelDocumentView(const std::shared_ptr<Document>& document,QWidget *parent = nullptr);
        ~ExcelDocumentView() override = default;

        void updateContent() override;
        bool saveContent() override;
        QWidget* widget() override;

    private:
        // TODO 我这里因为要引入xlnt，要将Qxlsx和OpenXlsx都清除掉，所以在这里注释一下
        void loadExcelFile(){};
        // void processWorksheet(const QXlsx::Document& xlsx);
        // QVariant getCellValue(const QXlsx::Cell* cell);

        std::shared_ptr<Document> document_;
        MergedTableView* tableView_;
        TableModel* model_;
    };
}
