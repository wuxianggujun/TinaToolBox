#include "MainWindow.hpp"
#include <QApplication>
#include <QFileDialog>
#include <QDebug>
#include <QMenu>
#include <QStackedWidget>
#include <QMdiSubWindow>
#include <QClipboard>
#include "LogPanel.hpp"
#include <QCloseEvent>
#include "DocumentArea.hpp"
#include "DocumentManager.hpp"
#include "FileHistory.hpp"
#include "ExceptionHandler.hpp"
#include "LineNumberTextEdit.hpp"
#include "PdfViewer.hpp"
#include "RecentFilesWidget.hpp"
#include "SimpleIni.h"
#include "StatusBar.hpp"
#include "TextDocumentView.hpp"

namespace TinaToolBox {
    MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent) {
        setWindowTitle(tr("TinaToolBox"));
        setMinimumSize(1024, 768);
        setWindowFlags(Qt::FramelessWindowHint);

        // 设置鼠标追踪
        setMouseTracking(true);

        centerWidget = new QWidget();
        setCentralWidget(centerWidget);

        mainLayout = new QVBoxLayout(centerWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
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
        rightSplitter->setHandleWidth(1); // 设置分割条宽度
        // rightSplitter->setChildrenCollapsible(false); // 防止子控件完全折叠

        documentArea = new DocumentArea();
        auto *centerSplitter = new QSplitter(Qt::Horizontal);
        centerSplitter->addWidget(documentArea);

        // 设置属性面板
        QWidget *propertyPanel = new QWidget();
        propertyPanel->setMinimumWidth(200);
        propertyPanel->setMaximumWidth(350);
        QVBoxLayout *propertyPanelLayout = new QVBoxLayout(propertyPanel);
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
        mainSplitter->setHandleWidth(1);
        mainSplitter->setCollapsible(0, false); // 防止左侧面板完全折叠
        mainSplitter->setChildrenCollapsible(false);

        // 设置分割器的样式
        QString splitterStyle = R"(
       QSplitter::handle {
           background-color: #e0e0e0;  /* 浅灰色 */
       }
       QSplitter::handle:hover {
           background-color: #007acc;  /* 保持悬停时的蓝色 */
       }
   )";

        statusBar = new StatusBar(this);
        mainLayout->addWidget(statusBar);

        mainSplitter->setStyleSheet(splitterStyle);
        rightSplitter->setStyleSheet(splitterStyle);
        centerSplitter->setStyleSheet(splitterStyle);

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
    }

    void MainWindow::mousePressEvent(QMouseEvent *event) {
        if (m_menuBar && m_menuBar->geometry().contains(event->pos())) {
            if (event->button() == Qt::LeftButton && !isMaximized()) {
                isDragging = true;
                dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
                event->accept();
            } else if (event->button() == Qt::LeftButton && isMaximized()) {
                // 如果在最大化状态下点击标题栏，恢复窗口并开始拖动
                showNormal();
                if (m_menuBar) {
                    m_menuBar->updateMaximizeButton(false);
                }
                // 计算新的拖动位置，使鼠标保持在点击的相对位置
                QPoint globalPos = event->globalPosition().toPoint();
                QRect geometry = frameGeometry();
                int clickX = event->pos().x();
                geometry.moveLeft(globalPos.x() - clickX);
                geometry.moveTop(globalPos.y() - m_menuBar->geometry().height() / 2);
                setGeometry(geometry);

                isDragging = true;
                dragPosition = QPoint(clickX, m_menuBar->geometry().height() / 2);
                event->accept();
            } else if (event->button() == Qt::RightButton) {
                // 禁用右键点击的默认行为
                event->accept();
                return;
            }
        }
        QMainWindow::mousePressEvent(event);
    }

    void MainWindow::mouseMoveEvent(QMouseEvent *event) {
        if (isDragging && (event->buttons() & Qt::LeftButton) && !isMaximized()) {
            move(event->globalPosition().toPoint() - dragPosition);
            event->accept();
        }
        QMainWindow::mouseMoveEvent(event);
    }

    void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
        isDragging = false;
        QMainWindow::mouseReleaseEvent(event);
    }

    void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton && m_menuBar && m_menuBar->geometry().contains(event->pos())) {
            // 如果窗口不是最大化状态，则最大化
            if (!isMaximized()) {
                showMaximized();
                if (m_menuBar) {
                    m_menuBar->updateMaximizeButton(true);
                }
            }
            event->accept();
            return;
        }
        QMainWindow::mouseDoubleClickEvent(event);
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
            auto document = manager.openDocument(filePath);
            if (document) {
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
        event->accept();
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

        leftPanelLayout->addWidget(recentFilesWidget);

        return leftPanel;
    }

    QWidget *MainWindow::createFileListToolBar() {
        auto *toolBar = new QWidget();
        toolBar->setStyleSheet(
            "QWidget {"
            "    background-color: #f5f5f5;"
            "    border-top: 1px solid #e0e0e0;"
            "}"
        );
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
}
