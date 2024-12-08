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
        } else {
            auto document = std::make_shared<Document>(path);
            documents_[path] = document;
            currentDocument_ = document;
            emit documentOpened(document);
        }
        emit currentDocumentChanged(currentDocument_);
        return currentDocument_;
    }

    void DocumentManager::closeDocument(const std::shared_ptr<Document> &document) {
        if (!document) return;

        QString filePath = document->filePath();
        spdlog::debug("Closing document: {}", filePath.toStdString());

        // 发出信号前先移除文档引用
        documents_.remove(filePath);
        // 发出信号
        emit documentClosed(document);
        if (currentDocument_ == document) {
            currentDocument_.reset();
        }
        emit currentDocumentChanged(currentDocument_);
        spdlog::debug("Document closed: {}", filePath.toStdString());
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

    QMap<QString, std::shared_ptr<Document>> DocumentManager::getDocuments() const {
        return documents_;
    }
}
