#include "DocumentArea.hpp"

#include "DocumentManager.hpp"
#include "DocumentView.hpp"
#include "DocumentViewFactory.hpp"

namespace TinaToolBox {
    DocumentArea::DocumentArea(QWidget *parent) {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        tabWidget_ = new DocumentTabWidget(this);
        tabWidget_->setDocumentMode(true);
        tabWidget_->setTabsClosable(true);
        layout->addWidget(tabWidget_);

        setupConnections();
    }

    DocumentArea::~DocumentArea() {
        // 清理所有资源
        qDeleteAll(documentViews_);
        documentViews_.clear();
    }

    void DocumentArea::onDocumentOpened(std::shared_ptr<Document> document) {
        if (!document) return;

        auto *view = createDocumentView(document);
        if (!view) return;

        documentViews_[document->filePath()] = view;
        tabWidget_->addTab(view, document->fileName());
        tabWidget_->setCurrentWidget(view);
    }

    void DocumentArea::onDocumentClosed(std::shared_ptr<Document> document) {
        if (!document) return;
        cleanupDocumentView(document->filePath());
    }

    void DocumentArea::onCurrentDocumentChanged(std::shared_ptr<Document> document) {
        if (!document) return;

        auto it = documentViews_.find(document->filePath());
        if (it != documentViews_.end()) {
            tabWidget_->setCurrentWidget(it.value());
        }
    }

    void DocumentArea::setupConnections() {
        auto &manager = DocumentManager::getInstance();
        connect(&manager, &DocumentManager::documentOpened, this, &DocumentArea::onDocumentOpened);
        connect(&manager, &DocumentManager::documentClosed, this, &DocumentArea::onDocumentClosed);
        connect(&manager, &DocumentManager::currentDocumentChanged, this, &DocumentArea::onCurrentDocumentChanged);

        connect(tabWidget_, &QTabWidget::tabCloseRequested, this, [this](int index) {
            if (auto *view = qobject_cast<DocumentView *>(tabWidget_->widget(index))) {
                DocumentManager::getInstance().closeDocument(view->getDocument());
            }
        });

        connect(tabWidget_, &QTabWidget::currentChanged, this, [this](int index) {
            if (auto *view = qobject_cast<DocumentView *>(tabWidget_->widget(index))) {
                DocumentManager::getInstance().setCurrentDocument(view->getDocument());
            }
        });
    }

    DocumentView *DocumentArea::createDocumentView(const std::shared_ptr<Document> &document) {
        auto *view = new DocumentView(document);
        auto docView = DocumentViewFactory::createDocumentView(document);
        if (docView) {
            view->setDocumentView(std::move(docView));
        }
        return view;
    }

    void DocumentArea::cleanupDocumentView(const QString &filePath) {
        auto it = documentViews_.find(filePath);
        if (it != documentViews_.end()) {
            int index = tabWidget_->indexOf(it.value());
            if (index != -1) {
                tabWidget_->removeTab(index);
            }
            delete it.value();
            documentViews_.erase(it);
        }
    }
}
