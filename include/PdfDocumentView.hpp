#pragma once

#include <memory>
#include "DocumentView.hpp"
#include "PdfViewer.hpp"

namespace TinaToolBox {

    class PdfDocumentView: public IDocumentView {
    public:
        explicit PdfDocumentView(const std::shared_ptr<Document>& document);
        ~PdfDocumentView() override;

        void updateContent() override;
        bool saveContent() override;
        QWidget* widget() override;

    private:
        void cleanup();
        std::shared_ptr<Document> document_;
        PdfViewer* pdfViewer_;
    };
    
}