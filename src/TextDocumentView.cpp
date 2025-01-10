#include "TextDocumentView.hpp"
#include <QTextBlock>
#include <QApplication>
#include <spdlog/spdlog.h>
#include <QTextCodec>
#include <QStringConverter>
#include <utility>
#include <QTextStream>
#include "EncodingDetector.hpp"
#include "utf8.h"

namespace TinaToolBox {
    TextDocumentView::TextDocumentView(std::shared_ptr<Document> document, QWidget *parent): QObject(parent),document_(std::move(
            document)), currentEncoding_("") {
        textEdit_ = new LineNumberTextEdit(parent);

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
            return;
        }

        currentEncoding_ = encoding;
        loadFileContext();
        emit encodingChanged(encoding);
    }

    QString TextDocumentView::getCurrentEncoding() const {
        return currentEncoding_;
    }

    QStringList TextDocumentView::availableEncodings() const
    {
       return {
         "UTF-8",
         "UTF-16LE",
         "UTF-16BE",
         "GBK",
         "GB18030",
         "GB2312",
         "Big5",
         "ASCII"
       };
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
            const ushort* unicodeData = reinterpret_cast<const ushort*>(data.constData());
            size_t numChars = data.size() / sizeof(ushort);
          if (currentEncoding_ == "UTF-16LE") {
               content = QString::fromUcs4(reinterpret_cast<const uint32_t*>(unicodeData), numChars);
            } else {
                // 针对 UTF-16BE 需要进行字节序交换
                QByteArray swappedData;
                swappedData.resize(data.size());
                for (size_t i = 0; i < numChars; ++i) {
                    swappedData[2 * i] = data[2 * i + 1];
                    swappedData[2 * i + 1] = data[2 * i];
                }
                content = QString::fromUcs4(reinterpret_cast<const uint32_t*>(swappedData.constData()), numChars);
            }
        } else if (currentEncoding_ == "ASCII") {
            content = QString::fromLatin1(data);
        } else {
            auto codec = QTextCodec::codecForName(currentEncoding_.toLatin1());
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
        if (!file.open(QIODevice::WriteOnly)) {
            spdlog::error("Failed to save file: {}", document_->filePath().toStdString());
            return false;
        }

        QString content = textEdit_->toPlainText();
        QByteArray data;

        if (currentEncoding_ == "UTF-8") {
            data = content.toUtf8();
        } else if (currentEncoding_ == "UTF-16LE") {
            const ushort* unicodeData = content.utf16();
            int size = content.size() * sizeof(ushort);
            data = QByteArray::fromRawData(reinterpret_cast<const char*>(unicodeData), size);
        } else if (currentEncoding_ == "UTF-16BE") {
            QByteArray utf16beData;
            const ushort* unicodeData = content.utf16();
            for (int i = 0; i < content.size(); ++i) {
                // 交换字节序
                utf16beData.append(static_cast<char>((unicodeData[i] >> 8) & 0xFF));
                utf16beData.append(static_cast<char>(unicodeData[i] & 0xFF));
            }
           data = utf16beData;
        } else if (currentEncoding_ == "ASCII") {
            data = content.toLatin1();
        } else {
            auto codec = QTextCodec::codecForName(currentEncoding_.toLatin1());
            if (codec) {
                data = codec->fromUnicode(content);
            } else {
                spdlog::error("Failed to find codec for encoding: {}", currentEncoding_.toStdString());
                return false;
            }
        }

        qint64 bytesWritten = file.write(data);
        if (bytesWritten == -1) {
            spdlog::error("Failed to write to file: {}", file.errorString().toStdString());
            file.close();
            return false;
        }
        if (bytesWritten != data.size()) {
            spdlog::error("Failed to write all data to file. Bytes written: {}, expected: {}", bytesWritten, data.size());
            file.close();
            return false;
        }

        file.close();
        textEdit_->document()->setModified(false);
        spdlog::debug("File saved successfully: {}", document_->filePath().toStdString());
        return true;
    }
}