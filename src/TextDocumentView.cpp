#include "TextDocumentView.hpp"

#include <QTextBlock>
#include <QApplication>
#include <spdlog/spdlog.h>
#include <QTextCodec>
#include <QStringConverter>  // Qt6 的新头文件
#include <utility>

#include "EncodingDetector.hpp"

namespace TinaToolBox {
    TextDocumentView::TextDocumentView(std::shared_ptr<Document> document, QWidget *parent): document_(std::move(document)),currentEncoding_("") {
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
        if (currentEncoding_ == encoding) {
            return;  // 如果编码相同，无需重新加载
        }

        currentEncoding_ = encoding;  // 更新当前编码
        loadFileContext();  // 重新加载文件内容
        emit encodingChanged(encoding);  // 发出编码变化信号
    }

    QString TextDocumentView::getCurrentEncoding() const {
        return currentEncoding_;
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


        if (currentEncoding_.isEmpty()) {
            // 自动检测编码
            currentEncoding_ = EncodingDetector::detect(data);
            spdlog::debug("Detected encoding for {}: {}", 
                          document_->filePath().toStdString(), 
                          currentEncoding_.toStdString());

            emit encodingChanged(currentEncoding_);
            
        }
        
        QString content;
        if (currentEncoding_ == "UTF-8") {
            content = QString::fromUtf8(data);
        } else if (currentEncoding_ == "UTF-16LE" || currentEncoding_ == "UTF-16BE") {
            content = QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData()));
        } else {
            QTextCodec *codec = QTextCodec::codecForName(currentEncoding_.toLatin1());
            if (codec) {
                content = codec->toUnicode(data);
            } else {
                spdlog::error("Failed to find codec for encoding: {}", currentEncoding_.toStdString());
                return false;
            }
        }
        textEdit_->setPlainText(content);
        textEdit_->document()->setModified(false);

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
