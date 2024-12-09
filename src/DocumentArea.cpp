#include "DocumentArea.hpp"

#include <QMessageBox>

#include "DocumentManager.hpp"
#include "DocumentView.hpp"
#include "DocumentViewFactory.hpp"

namespace TinaToolBox {
    DocumentArea::DocumentArea(QWidget *parent) : QWidget(parent) {
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

    DocumentView * DocumentArea::getCurrentDocumentView() const {
        if (auto* widget = tabWidget_->currentWidget()) {
            return qobject_cast<DocumentView*>(widget);
        }
        return nullptr;
    }

    void DocumentArea::onDocumentOpened(std::shared_ptr<Document> document) {
        if (!document) {
            spdlog::warn("Attempting to open null document");
            return;
        }

        auto *view = createDocumentView(document);
        if (!view) {
            spdlog::warn("Failed to create document view for document: {}", document->filePath().toStdString());
            return;
        }

        documentViews_[document->filePath()] = view;
        tabWidget_->addTab(view, document->fileName());
        tabWidget_->setCurrentWidget(view);

        updateTabState(view, document);
    }

    void DocumentArea::onDocumentClosed(std::shared_ptr<Document> document) {
        if (!document) return;
        cleanupDocumentView(document->filePath());
    }

    void DocumentArea::onDocumentStateChanged(std::shared_ptr<Document> document) {
        auto it = documentViews_.find(document->filePath());
        if (it == documentViews_.end()) return;

        updateTabState(it.value(), document);
    }

    void DocumentArea::onCurrentDocumentChanged(std::shared_ptr<Document> document) {
        if (!document) return;

        auto it = documentViews_.find(document->filePath());
        if (it != documentViews_.end()) {
            tabWidget_->setCurrentWidget(it.value());
        }
    }

    void DocumentArea::onDocumentError(std::shared_ptr<Document> document, const QString &error) {
        QMessageBox::warning(this, tr("Document Error"),
                             tr("Error in document %1:\n%2")
                             .arg(document->fileName())
                             .arg(error));
    }

    void DocumentArea::setupConnections() {
        auto &manager = DocumentManager::getInstance();
        connect(&manager, &DocumentManager::documentOpened, this, &DocumentArea::onDocumentOpened);
        connect(&manager, &DocumentManager::documentClosed, this, &DocumentArea::onDocumentClosed);
        connect(&manager, &DocumentManager::currentDocumentChanged, this, &DocumentArea::onCurrentDocumentChanged);

        // 标签页关闭请求
        connect(tabWidget_, &QTabWidget::tabCloseRequested, this, [this](int index) {
            if (auto *view = qobject_cast<DocumentView *>(tabWidget_->widget(index))) {
                DocumentManager::getInstance().closeDocument(view->getDocument());
            }
        });

        // 标签页切换
        connect(tabWidget_, &QTabWidget::currentChanged, this, [this](int index) {
            if (auto *view = qobject_cast<DocumentView *>(tabWidget_->widget(index))) {
                DocumentManager::getInstance().setCurrentDocument(view->getDocument());
            }
        });
    }

    DocumentView *DocumentArea::createDocumentView(const std::shared_ptr<Document> &document) {
        auto *view = new DocumentView(document);
        // 使用工厂创建具体的文档视图
        auto docView = DocumentViewFactory::createDocumentView(document);
        if (docView) {
            view->setDocumentView(std::move(docView));
        } else {
            spdlog::warn("No specific view created for document type: {}",
                         document->typeToString().toStdString());
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
            spdlog::debug("Cleaned up view for document: {}", filePath.toStdString());
        }
    }

    void DocumentArea::updateTabState(DocumentView *view, const std::shared_ptr<Document> &document) {
        int index = tabWidget_->indexOf(view);
        if (index == -1) return;

        switch (document->getState()) {
            case Document::State::Opening:
                // 显示加载中的状态
                tabWidget_->setTabIcon(index, QIcon(":/icons/loading.png"));
                tabWidget_->setTabToolTip(index, document->filePath());
                break;

            case Document::State::Ready:
                // 清除特殊图标，显示正常状态
                tabWidget_->setTabIcon(index, QIcon());
                tabWidget_->setTabToolTip(index, document->filePath());
                break;

            case Document::State::Error:
                // 显示错误状态
                tabWidget_->setTabIcon(index, QIcon(":/icons/error.png"));
                tabWidget_->setTabToolTip(index, document->lastError());
                break;
        }
    }
}
