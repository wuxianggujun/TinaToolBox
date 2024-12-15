#include "ScriptDocumentView.hpp"
#include <QTextBlock>
#include <QApplication>
#include <spdlog/spdlog.h>
#include <QTextCodec>
#include <QStringConverter>
#include <utility>

#include "EncodingDetector.hpp"

namespace TinaToolBox {
    ScriptDocumentView::ScriptDocumentView(std::shared_ptr<Document> document, QWidget *parent)
        :QObject(parent),document_(std::move(document)), currentEncoding_("") {
        codeEditor_ = new CodeEditor(parent);

        // 连接编辑器的修改信号
        connect(codeEditor_->document(), &QTextDocument::modificationChanged,
                this, &ScriptDocumentView::onTextModified);

        // 连接断点信号
        connect(codeEditor_, &CodeEditor::breakpointToggled,
                this, &ScriptDocumentView::onBreakpointToggled);

        loadFileContext();
    }

    ScriptDocumentView::~ScriptDocumentView() {
        spdlog::debug("ScriptDocumentView destroyed");
        delete codeEditor_;
        codeEditor_ = nullptr;
    }

    void ScriptDocumentView::updateContent() {
        loadFileContext();
    }

    bool ScriptDocumentView::saveContent() {
        return saveFileContext();
    }

    QWidget *ScriptDocumentView::widget() {
        return codeEditor_;
    }

    void ScriptDocumentView::onTextModified() {
        spdlog::info("Script modified");
    }

    void ScriptDocumentView::onBreakpointToggled(int line, bool added) {
        if (added) {
            spdlog::debug("Breakpoint added at line {}", line);
        } else {
            spdlog::debug("Breakpoint removed at line {}", line);
        }
        // 这里可以添加断点相关的处理逻辑
    }

    void ScriptDocumentView::setEncoding(const QString &encoding) {
        if (currentEncoding_ == encoding) {
            return;
        }

        currentEncoding_ = encoding;
        loadFileContext();
        emit encodingChanged(encoding);
    }

    QString ScriptDocumentView::getCurrentEncoding() const {
        return currentEncoding_;
    }

    bool ScriptDocumentView::loadFileContext() {
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
            currentEncoding_ = EncodingDetector::detect(data);
            spdlog::debug("Detected encoding: {}", currentEncoding_.toStdString());
            emit encodingChanged(currentEncoding_);
        }

        QString content;
        if (currentEncoding_ == "UTF-8") {
            content = QString::fromUtf8(data);
        } else if (currentEncoding_ == "ASCII") {
            content = QString::fromLatin1(data);
        } else {
            QTextCodec *codec = QTextCodec::codecForName(currentEncoding_.toLatin1());
            if (codec) {
                content = codec->toUnicode(data);
            } else {
                spdlog::error("Failed to find codec for encoding: {}", currentEncoding_.toStdString());
                return false;
            }
        }

        codeEditor_->setPlainText(content);
        codeEditor_->document()->setModified(false);
        return true;
    }

    bool ScriptDocumentView::saveFileContext() {
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
        out.setEncoding(QStringConverter::Utf8);
        try {
            QTextBlock block = codeEditor_->document()->begin();
            while (block.isValid()) {
                out << block.text();
                if (block.next().isValid()) {
                    out << "\n";
                }
                block = block.next();
                QApplication::processEvents(); // 保持UI响应
            }

            codeEditor_->document()->setModified(false);
            spdlog::debug("File saved successfully: {}", document_->filePath().toStdString());
            return true;
        } catch (const std::exception &e) {
            spdlog::error("Error saving file: {}", e.what());
            return false;
        }
    }
}