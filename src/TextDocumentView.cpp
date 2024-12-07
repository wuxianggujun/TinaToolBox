#include "TextDocumentView.hpp"

namespace TinaToolBox {
    TextDocumentView::TextDocumentView(std::shared_ptr<Document> document, QWidget *parent): document_(document) {
        textEdit_ = new LineNumberTextEdit(parent);

        // 连接文本编辑器的修改信号
        connect(textEdit_, &QPlainTextEdit::textChanged, this, &TextDocumentView::onTextChanged);
    }

    TextDocumentView::~TextDocumentView() {
        textEdit_->deleteLater();
    }

    void TextDocumentView::updateContent() {
        if (document_) {
            textEdit_->setPlainText(document_->content());
        }
    }

    bool TextDocumentView::saveContent() {
        if (document_) {
            document_->setContent(textEdit_->toPlainText());
            return document_->save();
        }
        return false;
    }

    QWidget *TextDocumentView::widget() {
        return textEdit_;
    }

    void TextDocumentView::onTextChanged() {
        if (document_) {
            document_->setContent(textEdit_->toPlainText());
        }
    }
}
