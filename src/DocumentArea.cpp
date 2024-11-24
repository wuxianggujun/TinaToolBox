//
// Created by wuxianggujun on 2024/11/24.
//

#include "DocumentArea.hpp"

#include <qfileinfo.h>
#include <QLabel>
#include "DocumentTabWidget.hpp"
#include "ExceptionHandler.hpp"

DocumentTab::DocumentTab(const QString &filePath, QWidget *parent): QWidget(parent), file_path_(filePath) {
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(0);

    stacked_widget_ = new QStackedWidget(this);
    layout_->addWidget(stacked_widget_);

    sheet_tab_ = new QTabWidget();
    sheet_tab_->setMaximumHeight(25);
    sheet_tab_->setTabPosition(QTabWidget::South);
    layout_->addWidget(sheet_tab_);
}

QTextEdit *DocumentTab::setupTextView() {
    if (!text_edit_) {
        text_edit_ = new QTextEdit(this);
        text_edit_->setReadOnly(false);
        text_edit_->setLineWrapMode(QTextEdit::NoWrap);
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

void DocumentTab::moveSheetTabs(bool showAtTop) {
}

void DocumentTab::setupToolBar(const QString &fileType) {
    toolbar_ = new QWidget();
    auto *toolbarLayout = new QHBoxLayout(toolbar_);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(4);

    toolbarLayout->addStretch();

    if (QStringList{"py", "js", "sh"}.contains(fileType.toLower())) {
        run_button_ = new RunButton(this);
        run_button_->setFixedSize(32, 32);
        toolbarLayout->addWidget(run_button_);

        /*connect(runButton, &RunButton::clicked, this, [this, filePath]() {
          if (auto* docTab = documents_.value(filePath)) {
              qDebug() << "Running script:" << filePath;
          }
      });*/
        connect(run_button_, &RunButton::clicked, this, &DocumentTab::runScript);
    }
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


/*DocumentArea::DocumentArea(QWidget *parent): QWidget(parent) {
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(0);
    
    // 创建标签页
    tab_widget_ = new QTabWidget();
    tab_widget_->setTabsClosable(true);
    tab_widget_->setMovable(true);
    tab_widget_->setDocumentMode(true);

    // 创建一个widget作为corner widget
    auto* cornerWidget = new QWidget();
    auto* cornerLayout = new QHBoxLayout(cornerWidget);
    cornerLayout->setContentsMargins(4, 0, 8, 0);
    cornerLayout->setSpacing(4);
    cornerLayout->setAlignment(Qt::AlignCenter);  // 居中对齐

    // 创建运行按钮
    auto* runButton = new RunButton(this);
    runButton->setFixedSize(20, 20);
    cornerLayout->addWidget(runButton);

    // 连接运行按钮的点击信号
    connect(runButton, &RunButton::clicked, this, [this]() {
        if (auto* currentTab = qobject_cast<DocumentTab*>(tab_widget_->currentWidget())) {
            qDebug() << "Running script:" << currentTab->getFilePath();
        }
    });
    
    // 设置corner widget
    tab_widget_->setCornerWidget(cornerWidget, Qt::TopRightCorner);
    
    tab_widget_->setStyleSheet(R"(
        QTabBar::tab {
            height: 35px;
            padding: 4px 8px;
        }
    )");

    connect(tab_widget_, &QTabWidget::tabCloseRequested,
            this, &DocumentArea::closeTab);

    layout_->addWidget(tab_widget_);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}*/

/*DocumentArea::DocumentArea(QWidget *parent): QWidget(parent) {
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(0);
    
    // 创建标签页
    tab_widget_ = new QTabWidget();
    tab_widget_->setTabsClosable(true);
    tab_widget_->setMovable(true);
    tab_widget_->setDocumentMode(true);

    // 创建一个widget作为corner widget
    auto* cornerWidget = new QWidget();
    cornerWidget->setFixedHeight(35);  // 设置固定高度与标签页一致
    auto* cornerLayout = new QHBoxLayout(cornerWidget);
    cornerLayout->setContentsMargins(4, 0, 8, 0);
    cornerLayout->setSpacing(4);

    // 创建运行按钮
    auto* runButton = new RunButton(this);
    runButton->setFixedSize(20, 20);
    
    // 设置布局的对齐方式
    cornerLayout->setAlignment(Qt::AlignVCenter);  // 垂直居中
    cornerLayout->addWidget(runButton, 0, Qt::AlignVCenter);  // 添加按钮时也指定垂直居中

    // 连接运行按钮的点击信号
    connect(runButton, &RunButton::clicked, this, [this]() {
        if (auto* currentTab = qobject_cast<DocumentTab*>(tab_widget_->currentWidget())) {
            qDebug() << "Running script:" << currentTab->getFilePath();
        }
    });
    
    // 设置corner widget
    tab_widget_->setCornerWidget(cornerWidget, Qt::TopRightCorner);
    
    tab_widget_->setStyleSheet(R"(
        QTabBar::tab {
            height: 35px;
            padding: 4px 8px;
        }
    )");

    connect(tab_widget_, &QTabWidget::tabCloseRequested,
            this, &DocumentArea::closeTab);

    layout_->addWidget(tab_widget_);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}*/

/*DocumentArea::DocumentArea(QWidget *parent): QWidget(parent) {
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(0);
    
    // 创建标签页
    tab_widget_ = new QTabWidget();
    tab_widget_->setTabsClosable(true);
    tab_widget_->setMovable(true);
    tab_widget_->setDocumentMode(true);

    // 创建一个空的占位标签页，以确保tabBar始终显示
    tab_widget_->addTab(new QWidget(), "");
    tab_widget_->removeTab(0);

    // 创建一个widget作为corner widget
    auto* cornerWidget = new QWidget();
    // cornerWidget->setFixedHeight(35);  // 设置固定高度与标签页一致
    auto* cornerLayout = new QHBoxLayout(cornerWidget);
    cornerLayout->setContentsMargins(4, 0, 8, 0);
    cornerLayout->setSpacing(4);
    
    // 创建运行按钮
    auto* runButton = new RunButton(this);
    runButton->setFixedSize(20, 20);
    cornerLayout->addWidget(runButton);
    cornerLayout->setAlignment(Qt::AlignVCenter);  // 设置垂直居中对齐
    // cornerLayout->addWidget(runButton, 0, Qt::AlignVCenter);

    // 设置cornerWidget的样式以确保正确的对齐
    cornerWidget->setStyleSheet(R"(
        QWidget {
            margin: 0;
            padding: 0;
        }
    )");

    
    // 连接运行按钮的点击信号
    connect(runButton, &RunButton::clicked, this, [this]() {
        if (auto* currentTab = qobject_cast<DocumentTab*>(tab_widget_->currentWidget())) {
            qDebug() << "Running script:" << currentTab->getFilePath();
        }
    });
    
    // 设置corner widget到标签栏的右侧
    tab_widget_->setCornerWidget(cornerWidget, Qt::TopRightCorner);
    
    // 设置样式，强制tabBar始终显示
    tab_widget_->setStyleSheet(R"(
        QTabWidget::pane {
            border: none;
        }
        QTabBar::tab {
            height: 35px;
            padding: 4px 8px;
        }
        QTabWidget::tab-bar {
            alignment: left;
        }
        QTabBar::scroller {
            width: 0px;
        }
    )");

    connect(tab_widget_, &QTabWidget::tabCloseRequested,
            this, &DocumentArea::closeTab);

    layout_->addWidget(tab_widget_);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}*/

DocumentArea::DocumentArea(QWidget *parent): QWidget(parent) {
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(0);
    
    // 创建标签页
    tab_widget_ = new DocumentTabWidget();
    
    connect(tab_widget_, &QTabWidget::tabCloseRequested,
            this, &DocumentArea::closeTab);

    layout_->addWidget(tab_widget_);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QWidget *DocumentArea::openDocument(const QString &filePath, const QString &fileType) {
    // 检查文档是否已经打开
    if (documents_.contains(filePath)) {
        int index = tab_widget_->indexOf(documents_[filePath]);
        tab_widget_->setCurrentIndex(index);
        return documents_[filePath];
    }

    // 创建新的文档标签
    auto *docTab = new DocumentTab(filePath, this);
    documents_[filePath] = docTab;

    // 添加到标签页
    QFileInfo fileInfo(filePath);
    tab_widget_->addDocumentTab(docTab,fileInfo.fileName());
    tab_widget_->setCurrentWidget(docTab);

    QWidget *view = nullptr;
    // 根据文件类型设置不同的视图
    if (QStringList{"xlsx", "xls"}.contains(fileType.toLower())) {
        view = docTab->setupExcelView();
    } else {
        view = docTab->setupTextView();
    }
    return view;
}

void DocumentArea::closeTab(int index) {
    QWidget *widget = tab_widget_->widget(index);
    QString filePath;

    // 查找对应的文件路径
    for (auto it = documents_.begin(); it != documents_.end(); ++it) {
        if (it.value() == widget) {
            filePath = it.key();
            break;
        }
    }

    if (!filePath.isEmpty()) {
        documents_.remove(filePath);
    }

    tab_widget_->removeDocumentTab(index);
    delete widget;
}
