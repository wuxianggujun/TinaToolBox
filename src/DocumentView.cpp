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
        // 先清理具体的文档视图
        if (documentView_) {
            documentView_.reset();
            documentView_ = nullptr;
        }
        document_.reset();
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
