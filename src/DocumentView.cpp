#include "DocumentView.hpp"

#include <QVBoxLayout>
#include <utility>
#include <spdlog/spdlog.h>
#include "Document.hpp"

namespace TinaToolBox {
    DocumentView::DocumentView(std::shared_ptr<Document> document) :QWidget(nullptr), document_(std::move(document)),documentView_(nullptr){
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0,0,0,0);
    }

    DocumentView::~DocumentView() {
        spdlog::debug("DocumentView destroyed: {}", 
        document_ ? document_->filePath().toStdString() : "null");
        documentView_.reset();
        document_.reset();
        documentView_ = nullptr;
        document_ = nullptr;
    }

    void DocumentView::setDocumentView(std::unique_ptr<IDocumentView> documentView) {
        if (documentView_) {
            layout()->removeWidget(documentView_->widget());
        }
        documentView_ = std::move(documentView);

        if (documentView_) {
            layout()->addWidget(documentView_->widget());
            documentView_->updateContent();
        }
    }

    std::shared_ptr<Document> DocumentView::getDocument() const {
        return document_;
    }
}
