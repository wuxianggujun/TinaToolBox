#pragma once

#include <QObject>
#include <memory>
#include "DocumentView.hpp"
#include "PdfViewer.hpp"

namespace TinaToolBox {

    class PdfDocumentView:public QObject, public IDocumentView {
        Q_OBJECT
    public:
        explicit PdfDocumentView(const std::shared_ptr<Document>& document,QWidget *parent = nullptr);
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