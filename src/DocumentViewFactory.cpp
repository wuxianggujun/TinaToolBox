#include "DocumentViewFactory.hpp"

#include "BlockProgrammingDocumentView.hpp"
#include "Document.hpp"
#include "ExcelDoucmentView.hpp"
#include "ScriptDocumentView.hpp"
#include "TextDocumentView.hpp"
#include "PdfDocumentView.hpp"

namespace TinaToolBox {
    std::unique_ptr<IDocumentView> DocumentViewFactory::createDocumentView(const std::shared_ptr<Document> &document) {
        switch (document->type()) {
            case Document::TEXT:
                return createTextView(document);
            case Document::PDF:
                return createPdfView(document);
            case Document::SCRIPT:
                return createBlockProgrammingView(document);
                // return createScriptView(document);
            case Document::EXCEL:
                return createExcelView(document);
            default:
                return nullptr;
        }
    }

    std::unique_ptr<IDocumentView> DocumentViewFactory::createTextView(const std::shared_ptr<Document> &document) {
        return std::make_unique<TextDocumentView>(document);
    }

    std::unique_ptr<IDocumentView> DocumentViewFactory::createPdfView(const std::shared_ptr<Document> &document) {
        return std::make_unique<PdfDocumentView>(document);
    }

    std::unique_ptr<IDocumentView> DocumentViewFactory::createScriptView(const std::shared_ptr<Document> &document) {
        return std::make_unique<ScriptDocumentView>(document);
    }

    std::unique_ptr<IDocumentView> DocumentViewFactory::createExcelView(const std::shared_ptr<Document> &document) {
        return std::make_unique<ExcelDocumentView>(document);
    }

    std::unique_ptr<IDocumentView> DocumentViewFactory::createBlockProgrammingView(
        const std::shared_ptr<Document> &document) {
        return std::make_unique<BlockProgrammingDocumentView>(document);
    }
}
