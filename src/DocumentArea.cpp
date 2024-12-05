//
// Created by wuxianggujun on 2024/11/24.
//

#include "DocumentArea.hpp"

#include <qfileinfo.h>
#include <QLabel>
#include <QTimer>
#include <utility>
#include "DocumentTabWidget.hpp"
#include "ExceptionHandler.hpp"

DocumentTab::DocumentTab(QString filePath, QWidget *parent): QWidget(parent), file_path_(std::move(filePath)) {
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(0);

    stacked_widget_ = new QStackedWidget(this);
    layout_->addWidget(stacked_widget_);

    sheet_tab_ = new QTabWidget();
    sheet_tab_->setMaximumHeight(35);
    sheet_tab_->setTabPosition(QTabWidget::South);
    layout_->addWidget(sheet_tab_);
}

void DocumentTab::moveSheetTabs(bool showAtTop) {
}

DocumentArea::DocumentArea(QWidget *parent): QWidget(parent) {
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(0);

    // 创建标签页
    tab_widget_ = new DocumentTabWidget();
    tab_widget_->tabBar()->setMinimumHeight(35);

    connect(tab_widget_, &QTabWidget::tabCloseRequested,
            this, &DocumentArea::closeFile);

    layout_->addWidget(tab_widget_);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

bool DocumentArea::openFile(const QString &filePath) {
    ExceptionHandler handler("打开文件失败");
    return handler([this,&filePath]() {
        if (openDocuments_.contains(filePath)) {
            int index = tab_widget_->indexOf(openDocuments_[filePath].view);
            tab_widget_->setCurrentIndex(index);
            emit currentFileChanged(filePath);
            return true;
        }
        // 创建新的文档视图和处理器
        QFileInfo fileInfo(filePath);
        QString extension = fileInfo.suffix().toLower();

        auto docHandler = DocumentHandlerFactory::createHandler(extension);
        if (!docHandler) {
            emit error(tr("不支持的文件类型:%1").arg(filePath));
            return false;
        }

        QWidget *view = createDocumentView(filePath);
        if (!view || !docHandler->loadDocument(view, filePath)) {
            if (view) view->deleteLater();
            emit error(tr("加载文件失败:%1").arg(filePath));
            return false;
        }

        // 存储文件路径作为视图的属性
        view->setProperty("filePath", filePath);

        DocumentInfo docInfo{view, docHandler};
        openDocuments_[filePath] = docInfo;

        // 设置标签页
        tab_widget_->addDocumentTab(view, fileInfo.fileName());
        tab_widget_->setCurrentWidget(view);

        emit fileOpened(filePath);

        return true;
    });
}

void DocumentArea::closeFile(int index) {
    if (index >= 0 && index < tab_widget_->count()) {
        QWidget *widget = tab_widget_->widget(index);
        QString filePath;

        // 查找对应的文件路径
        for (auto it = openDocuments_.begin(); it != openDocuments_.end(); ++it) {
            if (it.value().view == widget) {
                filePath = it.key();
                break;
            }
        }

        if (!filePath.isEmpty()) {
            // 先从tab移除，防止触发其他事件
            tab_widget_->removeDocumentTab(index);

            // 从映射中获取并移除文档信息
            auto docInfo = std::move(openDocuments_[filePath]);
            openDocuments_.remove(filePath);

            // 使用QTimer延迟清理资源，确保所有事件都处理完毕
            QTimer::singleShot(0, this, [docInfo = std::move(docInfo)]() mutable {
                if (docInfo.handler) {
                    docInfo.handler->cleanup(docInfo.view);
                }
            });
            emit fileClosed(filePath);
        }
    }
}


QString DocumentArea::getCurrentFilePath() const {
    QWidget *currentWidget = tab_widget_->currentWidget();
    return openDocuments_.keys().at(tab_widget_->indexOf(currentWidget));
}

QWidget *DocumentArea::getCurrentDocument() const {
    return tab_widget_->currentWidget();
}

void DocumentArea::closeAllDocuments() {
    for (int i = tab_widget_->count() - 1; i >= 0; --i) {
        if (canCloseDocument(i)) {
            closeFile(i);
        }
    }
}

bool DocumentArea::canCloseDocument(int index) const {
    return true;
}

QWidget *DocumentArea::createDocumentView(const QString &filePath) {
    QFileInfo file_info(filePath);
    QString extension = file_info.suffix().toLower();
    // 创建处理器
    auto handler = DocumentHandlerFactory::createHandler(extension);
    if (!handler) {
        return nullptr;
    }
    openDocuments_[filePath].handler = handler;
    return handler->createView(this);
}

void DocumentArea::showSettingsPanel() {
    if (!settingsPanel_) {
        settingsPanel_ = new SettingsPanel(this);
    }

    // 查找设置面板的索引
    int index = tab_widget_->indexOf(settingsPanel_);
    if (index >= 0) {
        tab_widget_->setCurrentIndex(index);
    } else {
        // 如果找不到，重新添加
        tab_widget_->addDocumentTab(settingsPanel_, "设置");
        tab_widget_->tabBar()->setTabButton(index, QTabBar::RightSide, nullptr);
    }
}
