#pragma once

#include <QObject>
#include <memory>
#include "Document.hpp"

/*管理文档的生命周期*/

namespace TinaToolBox {
    class DocumentManager : public QObject {
        Q_OBJECT

    public:
        static DocumentManager &getInstance();

        std::shared_ptr<Document> openDocument(const QString &path);

        void closeDocument(const std::shared_ptr<Document> &document);

        [[nodiscard]] std::shared_ptr<Document> getCurrentDocument() const;

        void setCurrentDocument(const std::shared_ptr<Document> &document);

        QMap<QString, std::shared_ptr<Document>> getDocuments() const;
        
    signals:
        void documentOpened(std::shared_ptr<Document> document);

        void documentClosed(std::shared_ptr<Document> document);

        void currentDocumentChanged(std::shared_ptr<Document> document);

    private:
        QMap<QString, std::shared_ptr<Document> > documents_;
        std::shared_ptr<Document> currentDocument_;
    };
}
