#pragma once

#include <QObject>
#include "Document.hpp"
#include "DocumentView.hpp"
#include "LineNumberTextEdit.hpp"

namespace TinaToolBox {
    class TextDocumentView : public QObject, public IDocumentView {
        Q_OBJECT

    public:
        explicit TextDocumentView(std::shared_ptr<Document> document, QWidget *parent = nullptr);

        ~TextDocumentView() override;

        void updateContent() override;

        bool saveContent() override;

        QWidget *widget() override;
    
    private:
        void onTextChanged();
        LineNumberTextEdit *textEdit_;
        std::shared_ptr<Document> document_;
    };
}
