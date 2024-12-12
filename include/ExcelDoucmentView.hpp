#pragma once

#include "Document.hpp"
#include "DocumentView.hpp"
#include "MergedTableView.hpp"
#include "TableModel.hpp"
#include <xlnt/xlnt.hpp>
#include <memory>

namespace TinaToolBox {
    class ExcelDocumentView : public IDocumentView {
    public:
        explicit  ExcelDocumentView(const std::shared_ptr<Document>& document);
        ~ExcelDocumentView() override = default;

        void updateContent() override;
        bool saveContent() override;
        QWidget* widget() override;

    private:
        void loadExcelFile();
        void processWorksheet(const xlnt::worksheet& worksheet);
        QVariant getCellValue(const xlnt::cell& cell);

        std::shared_ptr<Document> document_;
        MergedTableView* tableView_;
        TableModel* model_;
    };
}
