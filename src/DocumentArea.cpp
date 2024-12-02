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

QPlainTextEdit *DocumentTab::setupTextView() {
    if (!text_edit_) {
        text_edit_ = new LineNumberTextEdit(this);
        text_edit_->setObjectName("DocumentTabTextEdit"); // 添加这行
        text_edit_->setParent(this); // 显式设置父对象（虽然构造函数中已经设置了）
        text_edit_->setLineWrapMode(QPlainTextEdit::NoWrap);
        text_edit_->setStyleSheet(R"(
            QTextEdit {
                background-color: #1e1e1e;
                color: #d4d4d4;
                border: none;
                font-family: Consolas, 'Courier New', monospace;
                font-size: 12px;
            }
        )");

        stacked_widget_->addWidget(text_edit_);
        sheet_tab_->hide();
    }
    stacked_widget_->setCurrentWidget(text_edit_);
    return text_edit_;
}

MergedTableView *DocumentTab::setupExcelView() {
    try {
        if (!table_view_) {
            table_view_ = new MergedTableView(this);
            table_model_ = new TableModel(this);
            table_view_->setModel(table_model_);

            // 初始化Excel处理器
            excel_processor_ = std::make_unique<ExcelProcessor>();
            auto sheetsInfo = excel_processor_->readExcelStructure(file_path_);

            // 清空现有标签页
            sheet_tab_->clear();

            // 为每个sheet创建标签页
            for (const auto &sheetInfo: sheetsInfo) {
                auto *sheetWidget = new QWidget();
                sheet_tab_->addTab(sheetWidget, sheetInfo.sheetName);
            }

            // 加载第一个sheet的数据
            if (!sheetsInfo.empty()) {
                auto [data, mergedCells] = excel_processor_->readSheetData(0);
                if (!data.empty()) {
                    table_model_->setData(data, mergedCells);
                    if (!mergedCells.empty()) {
                        table_view_->setMergedCells(mergedCells);
                    }
                }
                sheet_tab_->setCurrentIndex(0);
            }

            sheet_tab_->show();
            connect(sheet_tab_, &QTabWidget::currentChanged,
                    this, &DocumentTab::changeSheet);

            stacked_widget_->addWidget(table_view_);
        }

        stacked_widget_->setCurrentWidget(table_view_);
        return table_view_;
    } catch (const std::exception &e) {
        spdlog::error("设置Excel视图失败: {}", e.what());
        return nullptr;
    }
}

PdfViewer *DocumentTab::setupPdfView() {
    if (!pdf_view_) {
        pdf_view_ = new PdfViewer(this);
        pdf_view_->setParent(this);
        stacked_widget_->addWidget(pdf_view_);
        sheet_tab_->hide(); // PDF不需要显示sheet标签页
    }
    stacked_widget_->setCurrentWidget(pdf_view_);
    return pdf_view_;
}

void DocumentTab::moveSheetTabs(bool showAtTop) {
}

void DocumentTab::changeSheet(int index) {
    if (index >= 0 && excel_processor_) {
        try {
            auto [data, mergedCells] = excel_processor_->readSheetData(index);
            if (!data.empty()) {
                table_model_->setData(data, mergedCells);
                if (!mergedCells.empty()) {
                    table_view_->setMergedCells(mergedCells);
                }
                table_view_->resizeColumnsToContents();
                table_view_->resizeRowsToContents();
            }
        } catch (const std::exception &e) {
            spdlog::error("切换sheet失败: {}", e.what());
        }
    }
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
        
        // 查找对应的文件路径
        QString filePath;
        for (auto it = openDocuments_.begin(); it != openDocuments_.end(); ++it) {
            if (it.value().view == widget) {
                filePath = it.key();
                break;
            }
        }

        if (!filePath.isEmpty()) {
            // 清理资源
            DocumentInfo &info = openDocuments_[filePath];
            if (info.handler) {
                info.handler->cleanup(info.view);
            }
            openDocuments_.remove(filePath);
        }
        // 移除标签页
        tab_widget_->removeTab(index);
        widget->deleteLater();

        emit fileClosed(filePath);
    }
}



QString DocumentArea::currentFilePath() const {
    QWidget *currentWidget = tab_widget_->currentWidget();
    return openDocuments_.keys().at(tab_widget_->indexOf(currentWidget));
}

QWidget *DocumentArea::currentView() const {
    return tab_widget_->currentWidget();
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


// void DocumentArea::closeTab(int index) {
//     ExceptionHandler handler("关闭标签页失败");
//     handler([this, index]() {
//         QWidget *widget = tab_widget_->widget(index);
//         if (!widget) {
//             spdlog::warn("Attempting to close null widget at index {}", index);
//             return false;
//         }
//
//         QString filePath;
//         // 查找对应的文件路径
//         for (auto it = documents_.begin(); it != documents_.end(); ++it) {
//             if (it.value() == widget) {
//                 filePath = it.key();
//                 break;
//             }
//         }
//
//         // 从映射中移除
//         if (!filePath.isEmpty()) {
//             documents_.remove(filePath);
//         }
//
//         // 先从标签页移除
//         tab_widget_->removeDocumentTab(index);
//
//         // 使用智能指针确保资源正确释放
//         std::shared_ptr<QWidget> widgetPtr(widget, [](QWidget *w) {
//             ExceptionHandler cleanupHandler("清理widget资源失败");
//             cleanupHandler([w]() {
//                 if (auto *docTab = qobject_cast<DocumentTab *>(w)) {
//                     if (auto *pdfViewer = docTab->getPdfViewer()) {
//                         pdfViewer->closeDocument();
//                     }
//                 }
//                 w->deleteLater();
//                 return true;
//             });
//         });
//
//         // 使用QTimer延迟删除
//         QTimer::singleShot(0, [widgetPtr]() {
//             // 智能指针会在这里自动调用删除器
//         });
//
//         return true;
//     });
// }
