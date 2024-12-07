#include "DocumentManager.hpp"

namespace TinaToolBox {
    DocumentManager & DocumentManager::getInstance() {
        static DocumentManager instance;
        return instance;
    }

    std::shared_ptr<Document> DocumentManager::openDocument(const QString &path) {
        if (documents_.contains(path)) {
            currentDocument_ = documents_[path];
        }else {
            auto document = std::make_shared<Document>(path);
            documents_[path] = document;
            currentDocument_ = document;
            emit documentOpened(document);
        }
        emit currentDocumentChanged(currentDocument_);
        return currentDocument_;
    }

    void DocumentManager::closeDocument(const std::shared_ptr<Document> &document) {
        if (document) {
            documents_.remove(document->filePath());
            emit documentClosed(document);

            if (currentDocument_ == document) {
                currentDocument_.reset();
                emit currentDocumentChanged(currentDocument_);
            }
        }
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
}
