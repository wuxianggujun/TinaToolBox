#pragma once

#include <QObject>
#include "Document.hpp"
#include "DocumentView.hpp"
#include "MergedTableView.hpp"
#include "TableModel.hpp"
#include <xlsxdocument.h>
#include  <xlsxcellrange.h>
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
        void loadExcelFile();
        void processWorksheet(const QXlsx::Document& xlsx);
        QVariant getCellValue(const QXlsx::Cell* cell);

        std::shared_ptr<Document> document_;
        MergedTableView* tableView_;
        TableModel* model_;
    };
}
