#pragma once

#include <QObject>
#include "Document.hpp"
#include "DocumentView.hpp"
#include "CodeEditor.hpp"

namespace TinaToolBox {
    class ScriptDocumentView : public QObject, public IDocumentView {
        Q_OBJECT

    public:
        explicit ScriptDocumentView(std::shared_ptr<Document> document, QWidget *parent = nullptr);
        ~ScriptDocumentView() override;

        void updateContent() override;
        bool saveContent() override;
        QWidget *widget() override;

        void setEncoding(const QString &encoding);
        [[nodiscard]] QString getCurrentEncoding() const;

        private slots:
            void onTextModified();
        void onBreakpointToggled(int line, bool added);

        signals:
            void encodingChanged(const QString &encoding);

    private:
        bool loadFileContext();
        bool saveFileContext();

        CodeEditor *codeEditor_;
        std::shared_ptr<Document> document_;
        static constexpr qint64 BLOCK_SIZE = 1024 * 1024; // 1MB per block

        QString currentEncoding_;
    };
}