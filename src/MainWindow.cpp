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
#include "FileHistory.hpp"
#include "ExceptionHandler.hpp"
#include "LineNumberTextEdit.hpp"
#include "PdfViewer.hpp"
#include "DocumentHandler.hpp"
#include "SimpleIni.h"


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

    loadFileHistory();
}

MainWindow::~MainWindow() {
    if (documentArea) {
        documentArea->closeAllDocuments();
    }
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

    auto *leftPanel = new QWidget();
    leftPanel->setMaximumWidth(400);
    leftPanel->setMinimumWidth(200);

    auto *leftPanelLayout = new QVBoxLayout(leftPanel);
    leftPanelLayout->setContentsMargins(0, 0, 0, 0);
    leftPanelLayout->setSpacing(0);

    leftPanelTab = new QTabWidget();
    leftPanelTab->setDocumentMode(true);
    leftPanelTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftPanelTab->setStyleSheet(R"(
        QTabBar::tab {
            height: 35px;  /* 与工具栏高度一致 */
        }
    )");
    leftPanelLayout->addWidget(leftPanelTab);

    fileTree = new QTreeWidget();
    fileTree->setHeaderLabels({"文件名", "修改日期", "类型", "大小"});
    fileTree->setColumnWidth(0, 200);
    fileTree->setColumnWidth(1, 150);
    fileTree->setColumnWidth(2, 80);
    fileTree->setColumnWidth(3, 100);
    fileTree->setIndentation(0);
    fileTree->setMouseTracking(true);

    scriptTree = new QTreeWidget();
    scriptTree->setHeaderHidden(true);

    leftPanelTab->addTab(fileTree, tr("文件"));
    leftPanelTab->addTab(scriptTree, tr("脚本"));
    // 设置标签栏自动扩展，使标签平分宽度
    leftPanelTab->tabBar()->setExpanding(true);

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

    mainSplitter->setStyleSheet(splitterStyle);
    rightSplitter->setStyleSheet(splitterStyle);
    centerSplitter->setStyleSheet(splitterStyle);

    installEventFilter(this);


    connect(fileTree, &QTreeWidget::itemEntered, this, &MainWindow::showFilePathToolTip);
    connect(fileTree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showFileTreeContextMenu);
    connect(logPanel, &LogPanel::closed, this, &MainWindow::hideBottomPanel);

    DocumentHandlerFactory::registerHandler(std::make_shared<PdfDocumentHandler>());
    DocumentHandlerFactory::registerHandler(std::make_shared<TextDocumentHandler>());
    
    setupConnections();
}

void MainWindow::createTileBar() {
    m_menuBar = new MainWindowMenuBar(this);
    setMenuWidget(m_menuBar); // 使用 setMenuWidget 而不是 setMenuBar

    // 连接信号
    connect(m_menuBar, &MainWindowMenuBar::minimizeClicked, this, &MainWindow::showMinimized);
    connect(m_menuBar, &MainWindowMenuBar::maximizeClicked, this, &MainWindow::toggleMaximize);
    connect(m_menuBar, &MainWindowMenuBar::closeClicked, this, &MainWindow::close);
    connect(m_menuBar, &MainWindowMenuBar::menuActionTriggered, this, &MainWindow::handleMenuAction);
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
            documentArea->openFile(filePath);
            updateFileHistory(filePath);
            updateUIState();
        }
    }
}

void MainWindow::onRunButtonStateChanged(bool isRunning) {

    FILE* fh = fopen(R"(C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\scripts\test.ttb)","r");
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

    for(const simpleparser::Token& currToken : tokens) {
        currToken.debugPrint();
    }
}

void MainWindow::loadFileHistory() {
    auto &fileHistoryManager = FileHistoryManager::getInstance(); // 使用 getInstance
    QVector<FileHistory> recentFiles = fileHistoryManager.getRecentFiles();

    // 清空现有的文件树
    fileTree->clear();

    for (const auto &file: recentFiles) {
        auto *item = new QTreeWidgetItem(fileTree);

        // 设置文件名
        item->setText(0, file.fileName);

        // 设置修改日期
        item->setText(1, file.modifiedDate.toString("yyyy-MM-dd HH:mm:ss"));

        // 设置文件类型
        item->setText(2, file.fileType.toUpper());

        // 设置文件大小
        QString sizeStr;
        if (file.fileSize < 1024) {
            sizeStr = QString("%1 B").arg(file.fileSize);
        } else if (file.fileSize < 1024 * 1024) {
            sizeStr = QString("%1 KB").arg(file.fileSize / 1024.0, 0, 'f', 2);
        } else {
            sizeStr = QString("%1 MB").arg(file.fileSize / 1024.0 / 1024.0, 0, 'f', 2);
        }
        item->setText(3, sizeStr);

        // 存储完整文件路径作为用户数据
        item->setData(0, Qt::UserRole, file.filePath);
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

void MainWindow::showFilePathToolTip(QTreeWidgetItem *item, int column) {
    if (item) {
        QString filePath = item->data(0, Qt::UserRole).toString();
        item->setToolTip(column, filePath);
    }
}

void MainWindow::showFileTreeContextMenu(const QPoint &pos) {
    QTreeWidgetItem *item = fileTree->itemAt(pos);
    if (!item) return;

    QMenu menu;
    QAction *copyAction = menu.addAction(tr("复制文件路径"));

    QAction *selectedAction = menu.exec(fileTree->viewport()->mapToGlobal(pos));

    if (selectedAction == copyAction) {
        QString filePath = item->data(0, Qt::UserRole).toString();
        QClipboard *clipboard = QApplication::clipboard();
        if (clipboard) {
            clipboard->setText(filePath);
        }
    }
}

void MainWindow::openFile() {
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("打开文件"),
        QString(),
        tr("所有文件 (*.*)")
    );
    if (filePath.isEmpty()) return;

    if (documentArea->openFile(filePath)) {
        updateFileHistory(filePath);
    }
}

void MainWindow::updateUIState() {
}

// 连接信号
void MainWindow::setupConnections() {
    connect(documentArea, &DocumentArea::error, this, [this](const QString &message) {
        QMessageBox::warning(this, tr("错误"), message);
    });

    connect(documentArea, &DocumentArea::fileOpened, this, &MainWindow::updateFileHistory);

    connect(documentArea->getTabWidget(), &DocumentTabWidget::runButtonStateChanged, this,
            &MainWindow::onRunButtonStateChanged);

    connect(documentArea, &DocumentArea::currentFileChanged, this, [this](const QString &filePath) {
        // 更新UI状态，如工具栏等
        updateUIState();
    });

    if (fileTree) {
        connect(fileTree, &QTreeWidget::itemDoubleClicked,
                this, &MainWindow::onFileDoubleClicked);
    }
}

void MainWindow::updateFileHistory(const QString &filePath) {
    if (filePath.isEmpty()) {
        return;
    }

    auto &fileHistoryManager = FileHistoryManager::getInstance(); // 使用 getInstance
    FileHistory existingRecord = fileHistoryManager.getFileHistory(filePath);
    if (existingRecord.filePath.isEmpty()) {
        if (!fileHistoryManager.addFileHistory(filePath)) {
            spdlog::error("Failed to add file to history: {}", filePath.toStdString());
            return;
        }
    } else {
        // 如果记录存在，更新记录
        if (!fileHistoryManager.updateFileHistory(filePath)) {
            spdlog::error("Failed to update file history: {}", filePath.toStdString());
            return;
        }
    }
    // 更新文件树显示
    updateFileTree();
}

void MainWindow::updateFileTree() {
    fileTree->clear();

    // 从数据库获取最近文件记录
    auto &fileHistoryManager = FileHistoryManager::getInstance(); // 使用 getInstance
    QVector<FileHistory> recentFiles = fileHistoryManager.getRecentFiles();

    for (const auto &file: recentFiles) {
        auto *item = new QTreeWidgetItem(fileTree);

        // 设置文件名
        item->setText(0, file.fileName);

        // 设置修改日期
        item->setText(1, file.modifiedDate.toString("yyyy-MM-dd HH:mm:ss"));

        // 设置文件类型
        item->setText(2, file.fileType.toUpper());

        // 设置文件大小
        QString sizeStr;
        if (file.fileSize < 1024) {
            sizeStr = QString("%1 B").arg(file.fileSize);
        } else if (file.fileSize < 1024 * 1024) {
            sizeStr = QString("%1 KB").arg(file.fileSize / 1024.0, 0, 'f', 2);
        } else {
            sizeStr = QString("%1 MB").arg(file.fileSize / 1024.0 / 1024.0, 0, 'f', 2);
        }
        item->setText(3, sizeStr);

        // 存储完整文件路径
        item->setData(0, Qt::UserRole, file.filePath);
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == this) {
        if (event->type() == QEvent::MouseMove) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            return false;
        } else if (event->type() == QEvent::Leave) {
            return false;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->accept();
}
