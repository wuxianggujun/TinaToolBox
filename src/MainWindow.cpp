#include "MainWindow.hpp"

#include <iostream>
#include <QApplication>
#include <QFileDialog>
#include <QDebug>
#include <QMenu>
#include <QStackedWidget>
#include <QMdiSubWindow>
#include <QClipboard>
#include "LogPanel.hpp"
#include <QCloseEvent>
#include <QPainterPath>
#include <QStandardPaths>
#include "DocumentArea.hpp"
#include "DocumentManager.hpp"
#include "FileHistory.hpp"
#include "ExceptionHandler.hpp"
#include "LineNumberTextEdit.hpp"
#include "LoadingProgressDialog.hpp"
#include "PdfViewer.hpp"
#include "RecentFilesWidget.hpp"
#include "SimpleIni.h"
#include "StatusBar.hpp"
#include "TextDocumentView.hpp"
#include "ConfigManager.hpp"
#include "ThemeManager.hpp"
#include "UIConfig.hpp"


namespace TinaToolBox {
    MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent) {
        // QString theme = ConfigManager::getInstance().getString("app","theme","light");

        setWindowTitle(tr("TinaToolBox"));
        setMinimumSize(1024, 768);
        setWindowFlags(Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        // 设置鼠标追踪
        setMouseTracking(true);

        centerWidget = new QWidget();
        setCentralWidget(centerWidget);

        mainLayout = new QVBoxLayout(centerWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        // mainLayout->setContentsMargins(SHADOW_WIDTH, SHADOW_WIDTH, SHADOW_WIDTH, SHADOW_WIDTH);
        mainLayout->setSpacing(0);

        setUpUI();
    }

    MainWindow::~MainWindow() {
        // 在程序退出时清理PDFium库
        PdfViewer::PDFiumLibrary::Destroy();
    }


    void MainWindow::setUpUI() {
        createTileBar();

        auto *mainContainer = new QWidget();
        mainLayout->addWidget(mainContainer);

        auto *mainContainerLayout = new QVBoxLayout(mainContainer);
        mainContainerLayout->setContentsMargins(0, 0, 0, 0);
        mainContainerLayout->setSpacing(0);

        // 创建主分割器
        mainSplitter = new QSplitter(Qt::Horizontal);
        mainContainerLayout->addWidget(mainSplitter);

        auto *leftPanel = createLeftPanel();

        mainSplitter->addWidget(leftPanel);

        rightSplitter = new QSplitter(Qt::Vertical);
        // rightSplitter->setChildrenCollapsible(false); // 防止子控件完全折叠

        documentArea = new DocumentArea();
        auto *centerSplitter = new QSplitter(Qt::Horizontal);
        centerSplitter->addWidget(documentArea);

        // 设置属性面板
        auto *propertyPanel = new QWidget();
        propertyPanel->setMinimumWidth(200);
        propertyPanel->setMaximumWidth(350);
        auto *propertyPanelLayout = new QVBoxLayout(propertyPanel);
        propertyPanelLayout->setContentsMargins(0, 0, 0, 0);
        propertyPanelLayout->setSpacing(0);

        propertyStack = new QStackedWidget();
        propertyPanelLayout->addWidget(propertyStack);

        centerSplitter->addWidget(propertyPanel);

        rightSplitter->addWidget(centerSplitter);

        // 创建底部面板
        bottomPanel = new QWidget();
        bottomPanel->setMinimumHeight(150);
        auto *bottomPanelLayout = new QVBoxLayout(bottomPanel);
        bottomPanelLayout->setContentsMargins(0, 0, 0, 0);
        bottomPanelLayout->setSpacing(0);

        // 添加日志面板
        logPanel = new LogPanel();
        bottomPanelLayout->addWidget(logPanel);
        rightSplitter->addWidget(bottomPanel);

        mainSplitter->addWidget(rightSplitter);

        rightSplitter->setSizes({700, 250});
        mainSplitter->setSizes({200, 800});


        // 设置分割器的拉伸因子
        rightSplitter->setStretchFactor(0, 1); // 上部分可以拉伸
        rightSplitter->setStretchFactor(1, 0); // 下部分不自动拉伸

        // 设置主分割器的属性
        mainSplitter->setCollapsible(0, false); // 防止左侧面板完全折叠
        mainSplitter->setChildrenCollapsible(false);


        statusBar = new StatusBar(this);
        mainLayout->addWidget(statusBar);

        installEventFilter(this);

        // connect(fileTree, &QTreeWidget::itemEntered, this, &MainWindow::showFilePathToolTip);
        // connect(fileTree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showFileTreeContextMenu);
        connect(logPanel, &LogPanel::closed, this, &MainWindow::hideBottomPanel);

        setupConnections();
    }

    void MainWindow::createTileBar() {
        m_menuBar = new MainWindowMenuBar(this);
        setMenuWidget(m_menuBar); // 使用 setMenuWidget 而不是 setMenuBar

        // 连接信号
        connect(m_menuBar, &MainWindowMenuBar::settingsClicked, this, &MainWindow::onSettingsClicked);
        connect(m_menuBar, &MainWindowMenuBar::minimizeClicked, this, &MainWindow::showMinimized);
        connect(m_menuBar, &MainWindowMenuBar::maximizeClicked, this, &MainWindow::toggleMaximize);
        connect(m_menuBar, &MainWindowMenuBar::closeClicked, this, &MainWindow::close);
        connect(m_menuBar, &MainWindowMenuBar::menuActionTriggered, this, &MainWindow::handleMenuAction);
    }

    void MainWindow::onSettingsClicked() {
        /*if (documentArea) {
            documentArea->showSettingsPanel();
        }*/

        // 使用 LogSystem 的实例引用，而不是直接使用 spdlog
        //auto& logSystem = LogSystem::getInstance();

        //logSystem.log("Settings button clicked", spdlog::level::info);
        // 2. 记录日志
        spdlog::info("Starting application...");
        qDebug() << "Debug information";
        std::cout << "Standard output" << std::endl;
    }

    void MainWindow::toggleMaximize() {
        if (isMaximized()) {
            showNormal();
            if (m_menuBar) {
                m_menuBar->updateMaximizeButton(false);
            }
        } else {
            showMaximized();
            if (m_menuBar) {
                m_menuBar->updateMaximizeButton(true);
            }
        }
    }

    void MainWindow::onFileDoubleClicked(const QTreeWidgetItem *item) {
        QString filePath = item->data(0, Qt::UserRole).toString();
        if (!filePath.isEmpty()) {
            if (documentArea) {
                DocumentManager::getInstance().openDocument(filePath);
                updateFileHistory(filePath);
            }
        }
    }

    void MainWindow::onRunButtonStateChanged(bool isRunning) {
        auto doc = TinaToolBox::DocumentManager::getInstance().getCurrentDocument();
        if (!doc || !doc->isScript()) {
            return;
        }

        /*if (isRunning) {
            // 处理脚本运行
            scriptRunner_->run(doc);
        } else {
            scriptRunner_->stop();
        }*/

        FILE *fh = fopen(R"(C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\scripts\test.ttb)", "r");
        if (!fh) {
            qDebug() << "Failed to open file";
            return;
        }
        fseek(fh, 0, SEEK_END);
        size_t fileSize = ftell(fh);
        fseek(fh, 0, SEEK_SET);
        std::string fileContents(fileSize, ' ');
        fread(fileContents.data(), 1, fileSize, fh);

        spdlog::info(fileContents);
        simpleparser::Tokenizer tokenizer;
        std::vector<simpleparser::Token> tokens = tokenizer.parse(fileContents);

        for (const simpleparser::Token &currToken: tokens) {
            currToken.debugPrint();
        }
    }

    void MainWindow::handleMenuAction(const QString &actionName) {
        qDebug() << "Execute menu action: " << actionName;
        if (actionName == "新建") {
            // Handle new file
        } else if (actionName == "打开") {
            openFile();
        } else if (actionName == "保存") {
            // Handle save
            *(volatile int *)0 = 0;
        } else if (actionName == "显示日志面板") {
            showBottomPanel();
        }
    }

    bool MainWindow::isTitleBarArea(const QPoint &pos) const {
        if (!titleBar) return false;
        return titleBar->geometry().contains(pos);
    }

    void MainWindow::showBottomPanel() {
        bottomPanel->show();
    }

    void MainWindow::hideBottomPanel() {
        bottomPanel->hide();
    }


    void MainWindow::openFile() {
        QString filePath = QFileDialog::getOpenFileName(
            this,
            tr("打开文件"),
            QString(),
            tr("所有文件 (*.*);;脚本文件 (*.ttd)")
        );
        if (!filePath.isEmpty()) {
            auto &manager = DocumentManager::getInstance();
            if (auto document = manager.openDocument(filePath)) {
                updateFileHistory(filePath);
                // 如果当前是脚本视图且打开的是非脚本文件，切换到所有文件视图
                if (viewModeComboBox->currentIndex() == 1 && !document->isScript()) {
                    viewModeComboBox->setCurrentIndex(0);
                }
            }
        }
    }

    // 连接信号
    void MainWindow::setupConnections() {
        connect(recentFilesWidget, &RecentFilesWidget::fileSelected,
                this, &MainWindow::onFileSelected);
        connect(recentFilesWidget, &RecentFilesWidget::removeFileRequested,
                this, &MainWindow::onRemoveFileRequested);
        // 连接 functionEntryClicked 信号到槽函数
        connect(recentFilesWidget, &RecentFilesWidget::functionEntryClicked,
                this, &MainWindow::onFunctionEntryClicked);

        // 监听文档变化
        connect(&DocumentManager::getInstance(), &DocumentManager::currentDocumentChanged,
                this, [this](const std::shared_ptr<Document> &document) {
                    if (document) {
                        statusBar->setFilePath(document->filePath());

                        // 获取当前文档视图
                        if (auto *docView = documentArea->getCurrentDocumentView()) {
                            if (auto *textView = dynamic_cast<TextDocumentView *>(docView->getDocumentView())) {
                                // 显示当前编码
                                statusBar->setEncoding(textView->getCurrentEncoding());
                                statusBar->setEncodingVisible(true);

                                // 断开所有之前的连接
                                disconnect(statusBar, &StatusBar::encodingChanged, nullptr, nullptr);
                                disconnect(textView, &TextDocumentView::encodingChanged, nullptr, nullptr);

                                // 重新建立双向连接
                                connect(statusBar, &StatusBar::encodingChanged,
                                        textView, &TextDocumentView::setEncoding,
                                        Qt::UniqueConnection);
                                connect(textView, &TextDocumentView::encodingChanged,
                                        statusBar, &StatusBar::setEncoding,
                                        Qt::UniqueConnection);

                                spdlog::debug("Connections established for document: {}",
                                              document->filePath().toStdString());
                            } else {
                                statusBar->setEncodingVisible(false);
                            }
                        }
                    } else {
                        statusBar->setFilePath("");
                        statusBar->setEncodingVisible(false);
                    }
                });
    }

    void MainWindow::updateFileHistory(const QString &filePath) {
        if (filePath.isEmpty()) return;

        recentFilesWidget->addRecentFile(filePath);
    }

    bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
        if (obj == this) {
            if (event->type() == QEvent::MouseMove) {
                auto *mouseEvent = dynamic_cast<QMouseEvent *>(event);
                return false;
            } else if (event->type() == QEvent::Leave) {
                return false;
            }
        }
        return QMainWindow::eventFilter(obj, event);
    }

    void MainWindow::closeEvent(QCloseEvent *event) {
        // 关闭所有打开的文档
        auto &manager = DocumentManager::getInstance();
        const auto documents = manager.getDocuments().values();

        for (const auto &doc: documents) {
            manager.closeDocument(doc);
        }
        // 等待所有清理完成
        QApplication::processEvents();
        event->accept();
    }

    void MainWindow::paintEvent(QPaintEvent *event) {
        Q_UNUSED(event);
        
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
    
        // 设置画笔和画刷
        painter.setPen(Qt::transparent);
        painter.setBrush(palette().window());

        // 获取窗口矩形
        QRectF rect = this->rect();

        if (isMaximized()) {
            // 最大化时绘制普通矩形
            painter.drawRect(rect);
        } else {
            // 非最大化时绘制圆角矩形
            int radius = UIConfig::getInstance().cornerRadius();
            painter.drawRoundedRect(rect, radius, radius);
        }
    }
    
    

    QWidget *MainWindow::createLeftPanel() {
        auto *leftPanel = new QWidget();
        leftPanel->setMaximumWidth(350);
        leftPanel->setMinimumWidth(200);

        auto *leftPanelLayout = new QVBoxLayout(leftPanel);
        leftPanelLayout->setContentsMargins(0, 0, 0, 0);
        leftPanelLayout->setSpacing(0);

        auto *toolBar = createFileListToolBar();

        leftPanelLayout->addWidget(toolBar);

        recentFilesWidget = new RecentFilesWidget();

        recentFilesWidget->addFunctionEntry("功能1");
        recentFilesWidget->addFunctionEntry("功能2");
        recentFilesWidget->addFunctionEntry("功能3");
        
        leftPanelLayout->addWidget(recentFilesWidget);

        return leftPanel;
    }

    QWidget *MainWindow::createFileListToolBar() {
        auto *toolBar = new QWidget();
        auto *toolBarLayout = new QHBoxLayout(toolBar);
        toolBarLayout->setContentsMargins(5, 2, 5, 2);

        auto *outputLabel = new QLabel("文件列表");
        outputLabel->setStyleSheet("color: #333333; font-weight: bold;");
        toolBarLayout->addWidget(outputLabel);

        viewModeComboBox = new QComboBox();
        viewModeComboBox->addItem("所有文件", "all");
        viewModeComboBox->addItem("脚本文件", "scripts");
        viewModeComboBox->setStyleSheet(
            "QComboBox {"
            "    background-color: white;"
            "    border: 1px solid #cccccc;"
            "    border-radius: 2px;"
            "    padding: 2px 5px;"
            "    min-width: 100px;"
            "}"
            "QComboBox:focus {"
            "    border: 1px solid #0078d7;"
            "}"
            "QComboBox::drop-down {"
            "    border: none;"
            "    width: 20px;"
            "}"
            "QComboBox::down-arrow {"
            "    width: 8px;"
            "    height: 8px;"
            "    background: none;"
            "    border-top: 2px solid #666;"
            "    border-right: 2px solid #666;"
            "    margin-top: -2px;"
            "}"
            "QComboBox QAbstractItemView {"
            "    border: 1px solid #cccccc;"
            "    selection-background-color: #e5f3ff;"
            "    selection-color: black;"
            "    background-color: white;"
            "    outline: 0px;"
            "}"
        );
        connect(viewModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this](int index) {
                    if (recentFilesWidget) {
                        recentFilesWidget->setShowScriptsOnly(index == 1); // 1 表示"脚本文件"选项
                    }
                });
        toolBarLayout->addWidget(viewModeComboBox);
        toolBarLayout->addStretch();
        return toolBar;
    }

    void MainWindow::onFileSelected(const QString &filePath) {
        if (!filePath.isEmpty()) {
            DocumentManager::getInstance().openDocument(filePath);
            updateFileHistory(filePath);
        }
    }

    void MainWindow::onRemoveFileRequested(const QString &filePath) {
        // 处理文件移除请求
        recentFilesWidget->removeRecentFile(filePath);
    }

    void MainWindow::onFunctionEntryClicked(const QString &functionName) {
        if (functionName == "功能1") {
            // 执行功能1的代码
            qDebug() << "执行功能1";
        } else if (functionName == "功能2") {
            // 执行功能2的代码
            qDebug() << "执行功能2";
        } else if (functionName == "功能3") {
            // 执行功能3的代码
            qDebug() << "执行功能3";
        }
    }
}
