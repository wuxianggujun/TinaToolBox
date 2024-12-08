#pragma once

#include <QWidget>
#include <memory>

namespace TinaToolBox {
    class Document;

    class IDocumentView {
    public:
        virtual ~IDocumentView() = default;
        virtual void updateContent() = 0;
        virtual bool saveContent() = 0;
        virtual QWidget* widget() = 0;
    };
    
    class DocumentView : public QWidget {
        Q_OBJECT
    public:
        explicit DocumentView(std::shared_ptr<Document> document);
        ~DocumentView() override;
        void setDocumentView(std::unique_ptr<IDocumentView> documentView);
        [[nodiscard]] IDocumentView* getDocumentView() const { return documentView_.get();}

        std::shared_ptr<Document> getDocument() const;
        
    protected:
        std::shared_ptr<Document> document_;
        std::unique_ptr<IDocumentView> documentView_;
    };
    
}
