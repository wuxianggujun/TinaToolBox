#include "TextDocumentView.hpp"

#include <QTextBlock>
#include <QApplication>
#include <spdlog/spdlog.h>
#include <QTextCodec>
#include <QStringConverter>  // Qt6 的新头文件

namespace TinaToolBox {
    TextDocumentView::TextDocumentView(std::shared_ptr<Document> document, QWidget *parent): document_(document) {
        textEdit_ = new LineNumberTextEdit(parent);

        // 连接文本编辑器的修改信号
        connect(textEdit_->document(), &QTextDocument::modificationChanged, this, &TextDocumentView::onTextModified);

        loadFileContext();
    }

    TextDocumentView::~TextDocumentView() {
        spdlog::debug("TextDocumentView destroyed");
        delete textEdit_;
        textEdit_ = nullptr;
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

    void TextDocumentView::setEncoding(const QString &encoding) {
        currentEncoding_ = encoding;
        loadFileContext(); // 重新加载文件内容
    }

    bool TextDocumentView::loadFileContext() {
        if (!document_) {
            spdlog::error("No document available");
            return false;
        }

        QFile file(document_->filePath());
        if (!file.open(QIODevice::ReadOnly)) {
            spdlog::error("Failed to open file: {}", document_->filePath().toStdString());
            return false;
        }

        QByteArray data = file.readAll();
        file.close();

        QString content;
        if (currentEncoding_ == "ANSI") {
            // 使用系统默认编码
            auto encoding = QStringConverter::System;
            auto decoder = QStringDecoder(encoding);
            content = decoder.decode(data);
        } else if (currentEncoding_ == "UTF-8") {
            auto decoder = QStringDecoder(QStringConverter::Utf8);
            content = decoder.decode(data);
        } else if (currentEncoding_ == "GB18030" || currentEncoding_ == "GBK" || currentEncoding_ == "GB2312") {
            // 对于中文编码，使用 System 编码
            auto decoder = QStringDecoder(QStringConverter::System);
            content = decoder.decode(data);
        } else if (currentEncoding_ == "Latin1") {
            auto decoder = QStringDecoder(QStringConverter::Latin1);
            content = decoder.decode(data);
        } else {
            // 默认使用系统编码
            auto decoder = QStringDecoder(QStringConverter::System);
            content = decoder.decode(data);
        }

        textEdit_->clear();
        textEdit_->setPlainText(content);
        textEdit_->document()->setModified(false);

        QTextCursor cursor = textEdit_->textCursor();
        cursor.movePosition(QTextCursor::Start);
        textEdit_->setTextCursor(cursor);

        spdlog::debug("File loaded with encoding {}: {}", 
                      currentEncoding_.toStdString(), 
                      document_->filePath().toStdString());
        return true;
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
