#include "MainWindow.hpp"

#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QFileDialog>
#include <QDebug>
#include <QMenu>
#include <QMenuBar>
#include <QStackedWidget>
#include <QMdiSubWindow>
#include <QPushButton>
#include <QClipboard>
#include "LogPanel.hpp"
#include <QCloseEvent>

#include "DocumentArea.hpp"
#include "FileHistory.hpp"


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
}

void MainWindow::setUpUI() {
    createTileBar();

    auto *mainContainer = new QWidget();
    mainLayout->addWidget(mainContainer);

    auto *mainContainerLayout = new QVBoxLayout(mainContainer);
    mainContainerLayout->setContentsMargins(0, 0, 0, 0);
    mainContainerLayout->setSpacing(0);

    // 创建主要的splitter
    mainSplitter = new QSplitter(Qt::Horizontal);
    mainContainerLayout->addWidget(mainSplitter);

    QWidget *leftPanel = new QWidget();
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
            background-color: #2d2d2d;
        }
        QSplitter::handle:hover {
            background-color: #007acc;
        }
    )";

    mainSplitter->setStyleSheet(splitterStyle);
    rightSplitter->setStyleSheet(splitterStyle);
    centerSplitter->setStyleSheet(splitterStyle);

    installEventFilter(this);


    connect(fileTree, &QTreeWidget::itemEntered, this, &MainWindow::showFilePathToolTip);
    connect(fileTree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showFileTreeContextMenu);
    connect(logPanel, &LogPanel::closed, this, &MainWindow::hideBottomPanel);

}

void MainWindow::createTileBar() {
    // 创建菜单栏
    m_menuBar = new MainWindowMenuBar(this);
    // 连接信号
    connect(m_menuBar, &MainWindowMenuBar::minimizeClicked, this, &MainWindow::showMinimized);
    connect(m_menuBar, &MainWindowMenuBar::maximizeClicked, this, &MainWindow::toggleMaximize);
    connect(m_menuBar, &MainWindowMenuBar::closeClicked, this, &MainWindow::close);
    // 添加这个连接
    connect(m_menuBar, &MainWindowMenuBar::menuActionTriggered, this, &MainWindow::handleMenuAction);
    mainLayout->addWidget(m_menuBar);
}


void MainWindow::loadFileHistory() {
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

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (m_menuBar->geometry().contains(event->pos())) {
            dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        if (!dragPosition.isNull()) {
            move(event->globalPosition().toPoint() - dragPosition);
            event->accept();
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    dragPosition = QPoint();
    QMainWindow::mouseReleaseEvent(event);
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

void MainWindow::showBottomPanel() {
    bottomPanel->show();
    logPanel->show();
    QList<int> sizes = rightSplitter->sizes();
    int total = sizes[0] + sizes[1];
    rightSplitter->setSizes({total - 200, 200});
}

void MainWindow::hideBottomPanel() {
    bottomPanel->hide();
    QList<int> sizes = rightSplitter->sizes();
    rightSplitter->setSizes({sizes[0] + sizes[1], 0});
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

    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        QString extension = fileInfo.suffix().toLower();

        // 保存文件历史到数据库
        FileHistoryManager fileHistoryManager;
        if (fileHistoryManager.addFileHistory(filePath)) {
            // 更新文件树显示
            updateFileTree();
        }

        // 根据文件类型打开文件
        if (extension == "xlsx" || extension == "xls") {
            // openExcelFile(filePath);
            // 暂时不处理Excel文件
        } else if (QStringList{"txt", "md", "py", "json", "xml", "yaml", "yml"}
            .contains(extension)) {
            openTextFile(filePath);
        } else {
            QMessageBox::warning(this, tr("警告"),
                                 tr("不支持的文件类型: %1").arg(extension));
            return;
        }
    }
}

void MainWindow::updateFileTree() {
    fileTree->clear();

    // 从数据库获取最近文件记录
    FileHistoryManager fileHistoryManager;
    QVector<FileHistory> recentFiles = fileHistoryManager.getRecentFiles();

    for (const auto &file: recentFiles) {
        QTreeWidgetItem *item = new QTreeWidgetItem(fileTree);

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

void MainWindow::openTextFile(const QString &filePath) {
    try {
        QWidget *widget = documentArea->openDocument(filePath, "text");
        QTextEdit *textEdit = qobject_cast<QTextEdit *>(widget);
        if (!textEdit) {
            throw std::runtime_error("Cannot cast to QTextEdit");
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Cannot open file");
        }

        QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        in.setEncoding(QStringConverter::Utf8);
#else
        in.setCodec("UTF-8");
#endif
        QString content = in.readAll();
        file.close();

        textEdit->setText(content);
    } catch (const std::exception &e) {
        QMessageBox::critical(this, tr("错误"),
                              tr("打开文本文件时出错: %1").arg(e.what()));
    }
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == this) {
        switch (event->type()) {
            case QEvent::MouseMove: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                return false; // 让事件继续传播
            }
            case QEvent::Leave: {
                return false; // 让事件继续传播
            }
            default:
                break;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Cleanup and save any necessary state
    if (logPanel) {
        logPanel->cleanup();
    }
    event->accept();
}
