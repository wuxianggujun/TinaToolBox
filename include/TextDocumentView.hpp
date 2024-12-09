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

        void setEncoding(const QString &encoding);
        
        [[nodiscard]] QString getCurrentEncoding() const;

    private slots:
        void onTextModified();

    signals:
        void encodingChanged(const QString &encoding);

    private:
        bool loadFileContext();

        bool saveFileContext();

        LineNumberTextEdit *textEdit_;
        std::shared_ptr<Document> document_;
        static constexpr qint64 BLOCK_SIZE = 1024 * 1024; // 1MB per block

        QString currentEncoding_{"UTF-8"};
    };
}
