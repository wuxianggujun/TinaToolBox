#include "DocumentManager.hpp"

#include <spdlog/spdlog.h>

namespace TinaToolBox {
    DocumentManager &DocumentManager::getInstance() {
        static DocumentManager instance;
        return instance;
    }

    std::shared_ptr<Document> DocumentManager::openDocument(const QString &path) {
        if (documents_.contains(path)) {
            currentDocument_ = documents_[path];
            auto doc = documents_[path];
            if (doc->getState() == Document::State::Ready) {
                currentDocument_ = doc;
                emit currentDocumentChanged(doc);
                return doc;
            }
        }

        auto document = std::make_shared<Document>(path);

        connect(document.get(), &Document::stateChanged, this, &DocumentManager::onDocumentStateChanged);
        connect(document.get(), &Document::errorOccurred, this, &DocumentManager::onDocumentError);

        documents_[path] = document;

        if (document->getState() == Document::State::Ready) {
            currentDocument_ = document;
            emit documentOpened(document);
            emit currentDocumentChanged(document);
        }
        return document;
    }

    void DocumentManager::closeDocument(const std::shared_ptr<Document> &document) {
        if (!document) return;

        QString filePath = document->filePath();
        spdlog::debug("Starting to close document: {}", filePath.toStdString());

        // 发出信号前先移除文档引用
        documents_.remove(filePath);
        // 发出信号
        emit documentClosed(document);
        if (currentDocument_ == document) {
            currentDocument_.reset();
            emit currentDocumentChanged(currentDocument_);
        }
        spdlog::debug("Document closed and cleaned up: {}", filePath.toStdString());
    }

    std::shared_ptr<Document> DocumentManager::getCurrentDocument() const {
        return currentDocument_;
    }

    void DocumentManager::setCurrentDocument(const std::shared_ptr<Document> &document) {
        if (currentDocument_ != document) {
            currentDocument_ = document;
            emit currentDocumentChanged(currentDocument_);
        }
    }

    QMap<QString, std::shared_ptr<Document> > DocumentManager::getDocuments() const {
        return documents_;
    }

    void DocumentManager::onDocumentStateChanged(Document::State state) {
        auto *document = qobject_cast<Document *>(sender());
        if (!document) return;

        // 查找对应的文档
        for (const auto& [path, doc] : documents_.asKeyValueRange()) {
            if (doc.get() == document) {
                emit documentStateChanged(doc);
                break;
            }
        }
    }

    void DocumentManager::onDocumentError(const QString &error) {
        auto *document = qobject_cast<Document *>(sender());
        if (!document) return;

        // 查找对应的文档
        for (const auto& [path, doc] : documents_.asKeyValueRange()) {
            if (doc.get() == document) {
                emit documentError(doc, error);
                break;
            }
        }
    }
}
