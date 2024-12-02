#include "DocumentHandler.hpp"
#include "PdfViewer.hpp"
#include "LineNumberTextEdit.hpp"
#include <QFile>


QList<std::shared_ptr<IDocumentHandler>> DocumentHandlerFactory::handlers_;


QWidget * PdfDocumentHandler::createView(QWidget *parent) {
    return new PdfViewer(parent);
}

bool PdfDocumentHandler::loadDocument(QWidget *view, const QString &filePath) {
    auto* pdfViewer = qobject_cast<PdfViewer*>(view);
    if (!pdfViewer) return false;
    return pdfViewer->loadDocument(filePath);
}

QString PdfDocumentHandler::getFileTypeName() const {
    return "PDF";
}

void PdfDocumentHandler::cleanup(QWidget *view) {
    if (auto* pdfViewer = qobject_cast<PdfViewer*>(view)) {
        // 先关闭PDF文档
        pdfViewer->closeDocument();

        if (pdfViewer) {
            pdfViewer->deleteLater();
        }
    }
}

QWidget * TextDocumentHandler::createView(QWidget *parent) {
    return new LineNumberTextEdit(parent);
}

bool TextDocumentHandler::loadDocument(QWidget *view, const QString &filePath) {
    auto* textEdit = qobject_cast<LineNumberTextEdit*>(view);
    if (!textEdit) return false;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);
#else
    in.setCodec("UTF-8");
#endif
    textEdit->setPlainText(in.readAll());
    return true;
}

QString TextDocumentHandler::getFileTypeName() const {
    return "TEXT";
}

void TextDocumentHandler::cleanup(QWidget *view) {
    if (view) {
        view->deleteLater();
    }
}

std::shared_ptr<IDocumentHandler> DocumentHandlerFactory::createHandler(const QString &fileType) {
    for (const auto& handler : handlers_) {
        if (handler->canHandle(fileType)) {
            return handler;
        }
    }
    return nullptr;
}

void DocumentHandlerFactory::registerHandler(std::shared_ptr<IDocumentHandler> handler) {
    handlers_.append(handler);
}

QStringList DocumentHandlerFactory::getSupportedFileTypes() {
    QStringList types;
    for (const auto& handler : handlers_) {
        types.append(handler->getFileTypeName());
    }
    return types;
}


