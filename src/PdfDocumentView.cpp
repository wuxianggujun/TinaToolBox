#include "PdfDocumentView.hpp"

#include "Document.hpp"

namespace TinaToolBox {
    PdfDocumentView::PdfDocumentView(const std::shared_ptr<Document> &document,QWidget *parent):QObject(parent),document_(document),
        pdfViewer_(new PdfViewer(parent)) {
    }

    PdfDocumentView::~PdfDocumentView() {
        cleanup();
    }

    void PdfDocumentView::updateContent() {
        if (!document_) {
            spdlog::error("No document set for PDF view");
            return;
        }

        if (!pdfViewer_->loadDocument(document_->filePath())) {
            spdlog::error("Failed to load PDF document: {}", document_->filePath().toStdString());
        }
    }

    bool PdfDocumentView::saveContent() {
        // PDF文档通常是只读的，不需要保存
        return true;
    }

    QWidget *PdfDocumentView::widget() {
        return pdfViewer_;
    }

    void PdfDocumentView::cleanup() {
        if (pdfViewer_) {
            // 确保先关闭文档
            pdfViewer_->closeDocument();
            // 删除 PdfViewer 实例
            delete pdfViewer_;
            pdfViewer_ = nullptr;
        }
        // 清理文档指针
        document_.reset();
    }
}
