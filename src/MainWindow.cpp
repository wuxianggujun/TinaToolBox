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
#include "DataFrame.hpp"
#include "ThemeManager.hpp"
#include "UIConfig.hpp"
#include <QElapsedTimer>
#include <QDebug>
#include <zlib.h>

#include "ExcelScriptInterpreter.hpp"
#include "ExcelHandler.hpp"
#include "ThreadPool.hpp"
#include "TTBFile.hpp"
#include "TTBScriptEngine.hpp"

namespace TinaToolBox
{
    MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
    {
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

    MainWindow::~MainWindow()
    {
        // 在程序退出时清理PDFium库
        PdfViewer::PDFiumLibrary::Destroy();
    }


    void MainWindow::setUpUI()
    {
        createTileBar();

        auto* mainContainer = new QWidget();
        mainLayout->addWidget(mainContainer);

        auto* mainContainerLayout = new QVBoxLayout(mainContainer);
        mainContainerLayout->setContentsMargins(0, 0, 0, 0);
        mainContainerLayout->setSpacing(0);

        // 创建主分割器
        mainSplitter = new QSplitter(Qt::Horizontal);
        mainContainerLayout->addWidget(mainSplitter);

        auto* leftPanel = createLeftPanel();

        mainSplitter->addWidget(leftPanel);

        rightSplitter = new QSplitter(Qt::Vertical);
        // rightSplitter->setChildrenCollapsible(false); // 防止子控件完全折叠

        documentArea = new DocumentArea();
        auto* centerSplitter = new QSplitter(Qt::Horizontal);
        centerSplitter->addWidget(documentArea);

        // 设置属性面板
        auto* propertyPanel = new QWidget();
        propertyPanel->setMinimumWidth(200);
        propertyPanel->setMaximumWidth(350);
        auto* propertyPanelLayout = new QVBoxLayout(propertyPanel);
        propertyPanelLayout->setContentsMargins(0, 0, 0, 0);
        propertyPanelLayout->setSpacing(0);

        propertyStack = new QStackedWidget();
        propertyPanelLayout->addWidget(propertyStack);

        centerSplitter->addWidget(propertyPanel);

        rightSplitter->addWidget(centerSplitter);

        // 创建底部面板
        bottomPanel = new QWidget();
        bottomPanel->setMinimumHeight(150);
        auto* bottomPanelLayout = new QVBoxLayout(bottomPanel);
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

    void MainWindow::createTileBar()
    {
        m_menuBar = new MainWindowMenuBar(this);
        setMenuWidget(m_menuBar); // 使用 setMenuWidget 而不是 setMenuBar

        // 连接信号
        connect(m_menuBar, &MainWindowMenuBar::settingsClicked, this, &MainWindow::onSettingsClicked);
        connect(m_menuBar, &MainWindowMenuBar::minimizeClicked, this, &MainWindow::showMinimized);
        connect(m_menuBar, &MainWindowMenuBar::maximizeClicked, this, &MainWindow::toggleMaximize);
        connect(m_menuBar, &MainWindowMenuBar::closeClicked, this, &MainWindow::close);
        connect(m_menuBar, &MainWindowMenuBar::menuActionTriggered, this, &MainWindow::handleMenuAction);
    }

    void MainWindow::onSettingsClicked()
    {
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

    void MainWindow::toggleMaximize()
    {
        if (isMaximized())
        {
            showNormal();
            if (m_menuBar)
            {
                m_menuBar->updateMaximizeButton(false);
            }
        }
        else
        {
            showMaximized();
            if (m_menuBar)
            {
                m_menuBar->updateMaximizeButton(true);
            }
        }
    }

    void MainWindow::onFileDoubleClicked(const QTreeWidgetItem* item)
    {
        QString filePath = item->data(0, Qt::UserRole).toString();
        if (!filePath.isEmpty())
        {
            if (documentArea)
            {
                DocumentManager::getInstance().openDocument(filePath);
                updateFileHistory(filePath);
            }
        }
    }

    void MainWindow::onRunButtonStateChanged(bool isRunning)
    {
        auto doc = TinaToolBox::DocumentManager::getInstance().getCurrentDocument();
        if (!doc || !doc->isScript())
        {
            return;
        }

        /*if (isRunning) {
            // 处理脚本运行
            scriptRunner_->run(doc);
        } else {
            scriptRunner_->stop();
        }*/

        FILE* fh = fopen(R"(C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\scripts\test.ttb)", "r");
        if (!fh)
        {
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

        for (const simpleparser::Token& currToken : tokens)
        {
            currToken.debugPrint();
        }
    }

    void MainWindow::handleMenuAction(const QString& actionName)
    {
        qDebug() << "Execute menu action: " << actionName;
        if (actionName == "新建")
        {
            // Handle new file
        }
        else if (actionName == "打开")
        {
            openFile();
        }
        else if (actionName == "保存")
        {
            // Handle save
        }
        else if (actionName == "显示日志面板")
        {
            showBottomPanel();
        }
    }

    bool MainWindow::isTitleBarArea(const QPoint& pos) const
    {
        if (!titleBar) return false;
        return titleBar->geometry().contains(pos);
    }

    void MainWindow::showBottomPanel()
    {
        bottomPanel->show();
    }

    void MainWindow::hideBottomPanel()
    {
        bottomPanel->hide();
    }


    void MainWindow::openFile()
    {
        QString filePath = QFileDialog::getOpenFileName(
            this,
            tr("打开文件"),
            QString(),
            tr("所有文件 (*.*);;脚本文件 (*.ttd)")
        );
        if (!filePath.isEmpty())
        {
            auto& manager = DocumentManager::getInstance();
            if (auto document = manager.openDocument(filePath))
            {
                updateFileHistory(filePath);
                // 如果当前是脚本视图且打开的是非脚本文件，切换到所有文件视图
                if (viewModeComboBox->currentIndex() == 1 && !document->isScript())
                {
                    viewModeComboBox->setCurrentIndex(0);
                }
            }
        }
    }

    // 连接信号
    void MainWindow::setupConnections()
    {
        connect(recentFilesWidget, &RecentFilesWidget::fileSelected,
                this, &MainWindow::onFileSelected);
        connect(recentFilesWidget, &RecentFilesWidget::removeFileRequested,
                this, &MainWindow::onRemoveFileRequested);
        // 连接 functionEntryClicked 信号到槽函数
        connect(recentFilesWidget, &RecentFilesWidget::functionEntryClicked,
                this, &MainWindow::onFunctionEntryClicked);

        // 监听文档变化
        connect(&DocumentManager::getInstance(), &DocumentManager::currentDocumentChanged,
                this, [this](const std::shared_ptr<Document>& document)
                {
                    if (document)
                    {
                        statusBar->setFilePath(document->filePath());

                        // 获取当前文档视图
                        if (auto* docView = documentArea->getCurrentDocumentView())
                        {
                            if (auto* textView = dynamic_cast<TextDocumentView*>(docView->getDocumentView()))
                            {
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
                            }
                            else
                            {
                                statusBar->setEncodingVisible(false);
                            }
                        }
                    }
                    else
                    {
                        statusBar->setFilePath("");
                        statusBar->setEncodingVisible(false);
                    }
                });
    }

    void MainWindow::updateFileHistory(const QString& filePath)
    {
        if (filePath.isEmpty()) return;

        recentFilesWidget->addRecentFile(filePath);
    }

    bool MainWindow::eventFilter(QObject* obj, QEvent* event)
    {
        if (obj == this)
        {
            if (event->type() == QEvent::MouseMove)
            {
                auto* mouseEvent = dynamic_cast<QMouseEvent*>(event);
                return false;
            }
            else if (event->type() == QEvent::Leave)
            {
                return false;
            }
        }
        return QMainWindow::eventFilter(obj, event);
    }

    void MainWindow::closeEvent(QCloseEvent* event)
    {
        // 关闭所有打开的文档
        auto& manager = DocumentManager::getInstance();
        const auto documents = manager.getDocuments().values();

        for (const auto& doc : documents)
        {
            manager.closeDocument(doc);
        }
        // 等待所有清理完成
        QApplication::processEvents();
        event->accept();
    }

    void MainWindow::paintEvent(QPaintEvent* event)
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 设置画笔和画刷
        painter.setPen(Qt::transparent);
        painter.setBrush(palette().window());

        // 获取窗口矩形
        QRectF rect = this->rect();

        if (isMaximized())
        {
            // 最大化时绘制普通矩形
            painter.drawRect(rect);
        }
        else
        {
            // 非最大化时绘制圆角矩形
            int radius = UIConfig::getInstance().cornerRadius();
            painter.drawRoundedRect(rect, radius, radius);
        }
    }


    QWidget* MainWindow::createLeftPanel()
    {
        auto* leftPanel = new QWidget();
        leftPanel->setMaximumWidth(350);
        leftPanel->setMinimumWidth(200);

        auto* leftPanelLayout = new QVBoxLayout(leftPanel);
        leftPanelLayout->setContentsMargins(0, 0, 0, 0);
        leftPanelLayout->setSpacing(0);

        auto* toolBar = createFileListToolBar();

        leftPanelLayout->addWidget(toolBar);

        recentFilesWidget = new RecentFilesWidget();

        recentFilesWidget->addFunctionEntry("功能1");
        recentFilesWidget->addFunctionEntry("功能2");
        recentFilesWidget->addFunctionEntry("功能3");

        leftPanelLayout->addWidget(recentFilesWidget);

        return leftPanel;
    }

    QWidget* MainWindow::createFileListToolBar()
    {
        auto* toolBar = new QWidget();
        auto* toolBarLayout = new QHBoxLayout(toolBar);
        toolBarLayout->setContentsMargins(5, 2, 5, 2);

        auto* outputLabel = new QLabel("文件列表");
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
                this, [this](int index)
                {
                    if (recentFilesWidget)
                    {
                        recentFilesWidget->setShowScriptsOnly(index == 1); // 1 表示"脚本文件"选项
                    }
                });
        toolBarLayout->addWidget(viewModeComboBox);
        toolBarLayout->addStretch();
        return toolBar;
    }

    void MainWindow::onFileSelected(const QString& filePath)
    {
        if (!filePath.isEmpty())
        {
            DocumentManager::getInstance().openDocument(filePath);
            updateFileHistory(filePath);
        }
    }

    void MainWindow::onRemoveFileRequested(const QString& filePath)
    {
        // 处理文件移除请求
        recentFilesWidget->removeRecentFile(filePath);
    }

    void MainWindow::onFunctionEntryClicked(const QString& functionName)
    {
        if (functionName == "功能1")
        {
            try
            {
                QElapsedTimer timer;
                timer.start();

                // 读取Excel文件
                qDebug() << "开始读取Excel文件...";
                auto df = DataFrame::fromExcel("C:/Users/wuxianggujun/Downloads/工单查询 (重复1.11).xlsx");
                qDebug() << "Excel文件读取完成，耗时:" << timer.elapsed() << "毫秒";

                qDebug() << "行数：" << df.rowCount() << "列数：" << df.columnCount();

                // 重置计时器用于计算数据处理时间
                timer.restart();

                // 数据处理
                for (const auto& name : df.getColumnNames())
                {
                    qDebug().noquote() << QString::fromStdString(name);
                }

                qDebug() << "数据处理完成，耗时:" << timer.elapsed() << "毫秒";
                qDebug() << "总耗时:" << timer.elapsed() << "毫秒";
            }
            catch (const std::exception& e)
            {
                qWarning() << "Error: " << e.what();
            }
        }
        if (functionName == "功能2")
        {
            try
            {
                TTBScriptEngine engine;

                // 设置进度回调
                engine.setProgressCallback([](const std::string& message, int progress)
                {
                    std::cout << "[" << progress << "%] " << message << std::endl;
                });

                // 设置配置更新回调
                engine.setConfigUpdateCallback([](const std::map<std::string, std::string>& config)
                {
                    std::cout << "Configuration updated:" << std::endl;
                    for (const auto& [key, value] : config)
                    {
                        std::cout << key << ": " << value << std::endl;
                    }
                });

                // 准备配置和脚本
                std::map<std::string, std::string> config = {
                    {"author", "John Doe"},
                    {"version", "1.0"},
                    {"description", "Excel automation script"},
                    {"target_file", "test.xlsx"},
                    {"last_modified", "2024-03-20"},
                    {"script_type", "excel_automation"}
                };

                std::string script = R"(
                    // 读取并显示配置
                    get config "target_file"
                    print config "author"
                    
                    // 设置新的配置
                    set config "last_run_date" "2024-03-20"
                    
                    // 使用配置值打开文件
                    open config "target_file"
                    select sheet 1
                    read A1
                    write "Hello" to B1
                    
                    // 显示更多配置信息
                    print config "version"
                    print config "description"
                )";

                // 使用默认密钥
                AESKey defaultKey = {
                    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
                };
                
                // 创建 TTB 文件
                std::string ttbFilename = "my_script_container.ttb";
                auto createResult = TTBFile::createEncrypted(ttbFilename, config, script, defaultKey, 
                                                           EncryptionFlags::AllEncrypted);

                if (!createResult) {
                    std::cerr << "Failed to create TTB script file" << std::endl;
                    return;
                }
                std::cout << "TTB file '" << ttbFilename.c_str() << "' created successfully." << std::endl;
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }

        if (functionName == "功能3")
        {
            try {
                // 1. 获取正确的文件路径
                QString appDir = QCoreApplication::applicationDirPath();
                QString templatePath = appDir + "/bin/TTBTemplate.exe";
                QString ttbPath = appDir + "/bin/my_script_container.ttb";
                QString outputPath = appDir + "/bin/output.exe";

                qDebug() << "Template path:" << templatePath;
                qDebug() << "TTB path:" << ttbPath;
                qDebug() << "Output path:" << outputPath;

                // 2. 首先复制模板文件到输出路径
                if (!QFile::copy(templatePath, outputPath)) {
                    if (QFile::exists(outputPath)) {
                        QFile::remove(outputPath);
                        if (!QFile::copy(templatePath, outputPath)) {
                            QMessageBox::critical(this, "Error", "Failed to create output file");
                            return;
                        }
                    }
                }

                // 3. 读取TTB文件数据
                QFile ttbFile(ttbPath);
                if (!ttbFile.open(QIODevice::ReadOnly)) {
                    QMessageBox::critical(this, "Error", "Failed to open TTB file");
                    return;
                }
                QByteArray ttbData = ttbFile.readAll();
                ttbFile.close();

                qDebug() << "Read TTB data size:" << ttbData.size() << "bytes";

                // 检查TTB数据是否有效
                if (ttbData.isEmpty()) {
                    QMessageBox::critical(this, "Error", "TTB file is empty");
                    return;
                }

                // 打印前几个字节用于调试
                qDebug() << "First few bytes of TTB data:";
                for (int i = 0; i < std::min<qsizetype>(16, ttbData.size()); ++i) {
                    qDebug().nospace() << QString("0x%1 ").arg((unsigned char)ttbData[i], 2, 16, QChar('0'));
                }

                // 压缩TTB数据
                uLong sourceLen = ttbData.size();
                uLong destLen = compressBound(sourceLen);
                std::vector<Bytef> compressedData(destLen);
                
                z_stream zs = {0};
                zs.zalloc = Z_NULL;
                zs.zfree = Z_NULL;
                zs.opaque = Z_NULL;
                
                int initResult = deflateInit2(&zs, 
                    Z_BEST_COMPRESSION,    // 压缩级别
                    Z_DEFLATED,           // 压缩方法
                    15 + 16,              // 窗口大小和gzip标记
                    8,                    // 内存级别
                    Z_DEFAULT_STRATEGY);   // 压缩策略
                
                if (initResult != Z_OK) {
                    QString error = QString("Failed to initialize compression: %1")
                        .arg(zs.msg ? zs.msg : "Unknown error");
                    QMessageBox::critical(this, "Error", error);
                    return;
                }
                
                zs.next_in = reinterpret_cast<Bytef*>(ttbData.data());
                zs.avail_in = sourceLen;
                zs.next_out = compressedData.data();
                zs.avail_out = destLen;
                
                // 修改压缩逻辑，处理缓冲区问题
                std::vector<Bytef> temp;
                while (true) {
                    int ret = deflate(&zs, Z_FINISH);
                    
                    if (ret == Z_STREAM_END) {
                        break;  // 压缩完成
                    }
                    
                    if (ret == Z_BUF_ERROR) {
                        // 输出缓冲区已满，需要扩展
                        size_t currentSize = compressedData.size();
                        temp = compressedData;  // 保存当前数据
                        compressedData.resize(currentSize * 2);  // 扩大一倍
                        std::copy(temp.begin(), temp.end(), compressedData.begin());
                        
                        // 更新zlib流的输出指针和大小
                        zs.next_out = compressedData.data() + zs.total_out;
                        zs.avail_out = compressedData.size() - zs.total_out;
                        
                        continue;
                    }
                    
                    if (ret < 0) {  // 其他错误
                        QString error = QString("Failed to compress data: %1 (ret=%2)")
                            .arg(zs.msg ? zs.msg : "Unknown error")
                            .arg(ret);
                        deflateEnd(&zs);
                        QMessageBox::critical(this, "Error", error);
                        qDebug() << "Compression error details:";
                        qDebug() << "Input size:" << sourceLen;
                        qDebug() << "Output buffer size:" << compressedData.size();
                        qDebug() << "Bytes written:" << zs.total_out;
                        qDebug() << "Return code:" << ret;
                        return;
                    }
                }
                
                compressedData.resize(zs.total_out);
                deflateEnd(&zs);

                qDebug() << "Compression successful:";
                qDebug() << "Original size:" << sourceLen << "bytes";
                qDebug() << "Compressed size:" << zs.total_out << "bytes";
                qDebug() << "Compression ratio:" << (float)zs.total_out / sourceLen * 100 << "%";

                // 打印压缩后数据的前几个字节
                qDebug() << "First few bytes of compressed data:";
                for (size_t i = 0; i < std::min<size_t>(16, compressedData.size()); ++i) {
                    qDebug().nospace() << QString("0x%1 ").arg((unsigned char)compressedData[i], 2, 16, QChar('0'));
                }

                // 4. 使用Windows API更新资源
                HANDLE hUpdateRes = BeginUpdateResourceW((LPCWSTR)outputPath.utf16(), FALSE);
                if (hUpdateRes == NULL) {
                    DWORD error = GetLastError();
                    QMessageBox::critical(this, "Error", 
                        QString("Failed to begin resource update. Error code: %1").arg(error));
                    return;
                }

                // 5. 添加TTB数据作为自定义资源
                if (!UpdateResourceW(hUpdateRes, 
                                   L"TTB_DATA",  // 资源类型
                                   MAKEINTRESOURCEW(1),  // 资源ID
                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                                   compressedData.data(),
                                   compressedData.size())) {
                    DWORD error = GetLastError();
                    EndUpdateResource(hUpdateRes, TRUE);  // 取消更改
                    QMessageBox::critical(this, "Error", 
                        QString("Failed to update resource. Error code: %1").arg(error));
                    return;
                }

                // 6. 提交资源更新
                if (!EndUpdateResource(hUpdateRes, FALSE)) {
                    DWORD error = GetLastError();
                    QMessageBox::critical(this, "Error", 
                        QString("Failed to commit resource update. Error code: %1").arg(error));
                    return;
                }

                // 7. 验证文件大小和资源
                QFile outputFile(outputPath);
                if (outputFile.exists()) {
                    qint64 finalSize = outputFile.size();
                    
                    // 验证资源是否正确写入
                    HANDLE hFile = CreateFileW((LPCWSTR)outputPath.utf16(), 
                                            GENERIC_READ, 
                                            FILE_SHARE_READ, 
                                            NULL, 
                                            OPEN_EXISTING, 
                                            FILE_ATTRIBUTE_NORMAL, 
                                            NULL);
                    
                    if (hFile != INVALID_HANDLE_VALUE) {
                        HMODULE hModule = LoadLibraryExW((LPCWSTR)outputPath.utf16(), 
                                                        NULL, 
                                                        LOAD_LIBRARY_AS_DATAFILE);
                        if (hModule) {
                            HRSRC hRes = FindResourceW(hModule, MAKEINTRESOURCEW(1), L"TTB_DATA");
                            if (hRes) {
                                DWORD resourceSize = SizeofResource(hModule, hRes);
                                qDebug() << "Verified resource size:" << resourceSize << "bytes";
                                if (resourceSize != compressedData.size()) {
                                    qWarning() << "Resource size mismatch! Original:" << compressedData.size() 
                                             << "Resource:" << resourceSize;
                                }
                            } else {
                                qWarning() << "Failed to verify resource!";
                            }
                            FreeLibrary(hModule);
                        }
                        CloseHandle(hFile);
                    }

                    QString msg = QString("Successfully created packed executable:\n%1\n\n"
                                        "Original TTB size: %2 bytes\n"
                                        "Final file size: %3 bytes")
                        .arg(outputPath)
                        .arg(ttbData.size())
                        .arg(finalSize);
                    QMessageBox::information(this, "Success", msg);
                }

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error", QString("Failed to pack executable: %1").arg(e.what()));
            }
        }
    }
}
