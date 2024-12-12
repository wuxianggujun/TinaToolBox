#pragma once
#include <memory>
#include "DocumentView.hpp"

/*负责创建对应类型的视图*/

namespace TinaToolBox {

    class DocumentViewFactory {
    public:
        static std::unique_ptr<IDocumentView> createDocumentView(const std::shared_ptr<Document>& document);

    private:
        static std::unique_ptr<IDocumentView> createTextView(const std::shared_ptr<Document>& document);
        static std::unique_ptr<IDocumentView> createPdfView(const std::shared_ptr<Document>& document);
        static std::unique_ptr<IDocumentView> createScriptView(const std::shared_ptr<Document>& document);
        static std::unique_ptr<IDocumentView> createExcelView(const std::shared_ptr<Document>& document);
    };
    
}