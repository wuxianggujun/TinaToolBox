#include "DocumentView.hpp"

#include <QVBoxLayout>
#include <utility>

TinaToolBox::DocumentView::DocumentView(std::shared_ptr<Document> document) :QWidget(nullptr), document_(std::move(document)),documentView_(nullptr){
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
}

void TinaToolBox::DocumentView::setDocumentView(std::unique_ptr<IDocumentView> documentView) {
    if (documentView_) {
        layout()->removeWidget(documentView_->widget());
    }
    documentView_ = std::move(documentView);

    if (documentView_) {
        layout()->addWidget(documentView_->widget());
        documentView_->updateContent();
    }
}

std::shared_ptr<TinaToolBox::Document> TinaToolBox::DocumentView::getDocument() const {
    return document_;
}
