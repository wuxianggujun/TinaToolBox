#include "DocumentViewFactory.hpp"

#include "Document.hpp"
#include "ScriptDocumentView.hpp"
#include "TextDocumentView.hpp"

namespace TinaToolBox {
    std::unique_ptr<IDocumentView> DocumentViewFactory::createDocumentView(const std::shared_ptr<Document> &document) {
        switch (document.get()->type()) {
            case Document::TEXT:
                return createTextView(document);
            case Document::PDF:
                return createPdfView(document);
            case Document::SCRIPT:
                return createScriptView(document);
            default:
                return nullptr;
        }
    }

    std::unique_ptr<IDocumentView> DocumentViewFactory::createTextView(const std::shared_ptr<Document> &document) {
        return std::make_unique<TextDocumentView>(document);
    }

    std::unique_ptr<IDocumentView> DocumentViewFactory::createPdfView(const std::shared_ptr<Document> &document) {
        // TODO
        return nullptr;
    }

    std::unique_ptr<IDocumentView> DocumentViewFactory::createScriptView(const std::shared_ptr<Document> &document) {
        return std::make_unique<ScriptDocumentView>(document);
    }
}
