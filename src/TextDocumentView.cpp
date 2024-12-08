#include "TextDocumentView.hpp"

#include <QTextBlock>
#include <QApplication>
#include <spdlog/spdlog.h>

namespace TinaToolBox {
    TextDocumentView::TextDocumentView(std::shared_ptr<Document> document, QWidget *parent): document_(document) {
        textEdit_ = new LineNumberTextEdit(parent);

        // 连接文本编辑器的修改信号
        connect(textEdit_->document(), &QTextDocument::modificationChanged, this, &TextDocumentView::onTextModified);

        loadFileContext();
    }

    TextDocumentView::~TextDocumentView() {
        textEdit_->deleteLater();
    }

    void TextDocumentView::updateContent() {
        loadFileContext();
    }

    bool TextDocumentView::saveContent() {
        return saveFileContext();
    }

    QWidget *TextDocumentView::widget() {
        return textEdit_;
    }

    void TextDocumentView::onTextModified() {
        spdlog::info("Text modified");
    }

    bool TextDocumentView::loadFileContext() {
        if (!document_) {
            spdlog::error("No document available");
            return false;
        }

        QFile file(document_->filePath());

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            spdlog::error("Failed to open file: {}", document_->filePath().toStdString());
            return false;
        }

        textEdit_->clear();

        QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        in.setEncoding(QStringConverter::Utf8);
#else
        in.setCodec("UTF-8");
#endif

        QTextCursor cursor(textEdit_->document());
        cursor.beginEditBlock();

        try {
            while (!in.atEnd()) {
                QString block = in.read(BLOCK_SIZE);
                cursor.insertText(block);
                QApplication::processEvents(); // 保持UI响应
            }

            cursor.endEditBlock();

            textEdit_->document()->setModified(false);

            cursor.movePosition(QTextCursor::Start);
            textEdit_->setTextCursor(cursor);

            spdlog::debug("File loaded successfully: {}", document_->filePath().toStdString());
            return true;
        } catch (const std::exception &e) {
            spdlog::error("Error loading file: {}", e.what());
            cursor.endEditBlock();
            return false;
        }
    }

    bool TextDocumentView::saveFileContext() {
        if (!document_) {
            spdlog::error("No document available");
            return false;
        }

        QFile file(document_->filePath());

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            spdlog::error("Failed to save file: {}", document_->filePath().toStdString());
            return false;
        }

        QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        out.setEncoding(QStringConverter::Utf8);
#else
        out.setCodec("UTF-8");
#endif
        try {
            QTextBlock block = textEdit_->document()->begin();

            while (block.isValid()) {
                out << block.text();

                if (block.next().isValid()) {
                    out << "\n";
                }

                block = block.next();
                QApplication::processEvents(); // 保持UI响应
            }

            textEdit_->document()->setModified(false);
            spdlog::debug("File saved successfully: {}", document_->filePath().toStdString());
            return true;
        } catch (const std::exception &e) {
            spdlog::error("Error saving file: {}", e.what());
            return false;
        }
    }
}
