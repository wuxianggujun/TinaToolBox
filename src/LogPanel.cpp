//
// Created by wuxianggujun on 2024/11/23.
//

#include "LogPanel.hpp"

#include <QDateTime>
#include <QTextBlock>
#include <QScrollBar>
#include <QTextCursor>
#include <QTimer>
#include <spdlog/pattern_formatter.h>

LogPanel *LogPanel::instance_ = nullptr;

template<typename Mutex>
void LogPanelSink<Mutex>::sink_it_(const spdlog::details::log_msg &msg) {
    if (!panel_) return;

    // 设置日志格式，包含时间戳
    if (!spdlog::sinks::base_sink<Mutex>::formatter_) {
        spdlog::sinks::base_sink<Mutex>::set_formatter(
            std::make_unique<spdlog::pattern_formatter>("[%Y-%m-%d %H:%M:%S.%e] %v"));
    }
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    QString text = QString::fromUtf8(formatted.data(), static_cast<int>(formatted.size()));

    // 根据日志级别设置颜色
    QColor color;
    switch (msg.level) {
        case spdlog::level::trace:
            color = Qt::gray;
            break;
        case spdlog::level::debug:
            color = Qt::darkGreen;
            break;
        case spdlog::level::info:
            color = Qt::black;
            break;
        case spdlog::level::warn:
            color = QColor(255, 165, 0); // Orange
            break;
        case spdlog::level::err:
            color = Qt::red;
            break;
        case spdlog::level::critical:
            color = QColor(139, 0, 0); // Dark red
            break;
        default:
            color = Qt::black;
    }

    // 使用 Qt::QueuedConnection 确保在主线程中更新 UI
    QMetaObject::invokeMethod(panel_, "logMessage", Qt::QueuedConnection,
                              Q_ARG(QString, text), Q_ARG(QColor, color));
}

void LogPanel::onLogLevelChanged(int index) {
    currentLogLevel_ = logLevelComboBox_->currentData().toInt();
   
    // 使用 QTimer 来延迟执行过滤，避免界面卡死
    QTimer::singleShot(0, this, &LogPanel::filterLogsByLevel);
}

void LogPanel::filterLogsByLevel() {
    // 清空当前显示
    logArea_->clear();
    // 获取当前选择的日志级别
    int selectedLevel = logLevelComboBox_->currentData().toInt();

    spdlog::debug("当前选择的日志级别: {}", selectedLevel);

    for (const LogEntry& entry : logEntries_) {
        bool shouldShow = false;

        if (selectedLevel == -1) {  // ALL
            shouldShow = true;
        }
        else {
            // spdlog的级别：trace=0, debug=1, info=2, warn=3, error=4, critical=5
            shouldShow = (entry.level == selectedLevel);
        }

        if (shouldShow) {
            logArea_->setTextColor(entry.color);
            logArea_->append(entry.text);
        }
    }
}

void LogPanel::qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if (!instance_) return;
    // 获取当前时间并格式化
    auto now = QDateTime::currentDateTime();
    auto timestamp = now.toString("[yyyy-MM-dd HH:mm:ss.zzz] ");
    QColor color;
    QString prefix;
    switch (type) {
        case QtDebugMsg:
            color = Qt::darkGreen;
            prefix = "[Debug] ";
            break;
        case QtInfoMsg:
            color = Qt::black;
            prefix = "[Info] ";
            break;
        case QtWarningMsg:
            color = QColor(255, 165, 0);
            prefix = "[Warning] ";
            break;
        case QtCriticalMsg:
            color = Qt::red;
            prefix = "[Critical] ";
            break;
        case QtFatalMsg:
            color = QColor(139, 0, 0);
            prefix = "[Fatal] ";
            break;
    }

    QString logMessage = timestamp + prefix + msg;
    if (context.file) {
        logMessage += QString(" (%1:%2)").arg(context.file).arg(context.line);
    }

    QMetaObject::invokeMethod(instance_, "logMessage", Qt::QueuedConnection,
                              Q_ARG(QString, logMessage), Q_ARG(QColor, color));
}

LogPanel::LogPanel(QWidget *parent) : QWidget(parent) {
    instance_ = this;
    // 初始化当前日志级别为 ALL (-1)
    currentLogLevel_ = -1;
    setupUI();
    setupLogHandlers();

    // 确保 ComboBox 初始选择为 "All"
    logLevelComboBox_->setCurrentIndex(0);

    // 连接信号和槽
    connect(this, &LogPanel::logMessage, this, [this](const QString &message, const QColor &color) {
        appendLogWithColor(message, color);
    });
}

LogPanel::~LogPanel() {
    if (instance_ == this) {
        instance_ = nullptr;
    }
    // 清理spdlog sink
    if (sink_) {
        spdlog::drop_all(); // 清理所有logger
        sink_.reset();
    }
    // 恢复默认的消息处理器
    qInstallMessageHandler(nullptr);
}

void LogPanel::setupLogHandlers() {
    // 创建并注册自定义sink
    sink_ = std::make_shared<LogPanelSink<std::mutex> >(this);
    auto logger = spdlog::default_logger();
    logger->sinks().clear();
    logger->sinks().push_back(sink_);

    // 设置Qt消息处理器
    qInstallMessageHandler(qtMessageHandler);
}

void LogPanel::appendLog(const QString &text) {
    logArea_->append(text);
}

void LogPanel::appendLogWithColor(const QString &text, const QColor &color) {
    // 解析日志级别
    spdlog::level::level_enum level = spdlog::level::info;  // 默认级别
    // 更精确的日志级别检测
    QString lowerText = text.toLower();
    if (lowerText.contains("[trace]")) {
        level = spdlog::level::trace;
    }
    else if (lowerText.contains("[debug]")) {
        level = spdlog::level::debug;
    }
    else if (lowerText.contains("[info]")) {
        level = spdlog::level::info;
    }
    else if (lowerText.contains("[warning]") || lowerText.contains("[warn]")) {
        level = spdlog::level::warn;
    }
    else if (lowerText.contains("[error]") || lowerText.contains("[err]")) {
        level = spdlog::level::err;
    }
    else if (lowerText.contains("[critical]")) {
        level = spdlog::level::critical;
    }

    // 存储日志条目
    logEntries_.append({text, color, level});

    // 检查是否应该显示这条日志
    int currentLevel = logLevelComboBox_->currentData().toInt();
    bool shouldShow = (currentLevel == -1) || (level == currentLevel);

    if (shouldShow) {
        logArea_->setTextColor(color);
        logArea_->append(text);
    }
}

// 在头文件中添加日志级别的调试辅助函数
QString getLevelName(spdlog::level::level_enum level) {
    switch (level) {
    case spdlog::level::trace: return "TRACE";
    case spdlog::level::debug: return "DEBUG";
    case spdlog::level::info: return "INFO";
    case spdlog::level::warn: return "WARN";
    case spdlog::level::err: return "ERROR";
    case spdlog::level::critical: return "CRITICAL";
    default: return "UNKNOWN";
    }
}
void LogPanel::clearLog() {
    logArea_->clear();
    logEntries_.clear();  // 同时清除存储的日志
}

void LogPanel::closePanel() {
    emit closed();
}

void LogPanel::onSearchTextChanged(const QString &text) {
    if (text.isEmpty()) {
        // 清除所有高亮
        QTextCursor cursor = logArea_->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        QTextCharFormat format;
        format.setBackground(Qt::transparent);
        cursor.mergeCharFormat(format);
        return;
    }

    // 高亮匹配的文本
    QTextCursor cursor(logArea_->document());
    QTextCharFormat highlightFormat;
    highlightFormat.setBackground(Qt::yellow);

    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor = logArea_->document()->find(text, cursor);
        if (!cursor.isNull()) {
            cursor.mergeCharFormat(highlightFormat);
        }
    }
}


void LogPanel::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *toolbar = new QWidget();
    toolbar->setStyleSheet(
        "QWidget {"
        "    background-color: #f5f5f5;"
        "    border-top: 1px solid #e0e0e0;"
        "}"
    );

    auto *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(5, 2, 5, 2);

    auto *outputLabel = new QLabel("输出");
    outputLabel->setStyleSheet("color: #333333; font-weight: bold;");
    toolbarLayout->addWidget(outputLabel);

    logLevelComboBox_ = new QComboBox();
    logLevelComboBox_->addItem("All", -1);
    logLevelComboBox_->addItem("Trace", spdlog::level::trace);
    logLevelComboBox_->addItem("Debug", spdlog::level::debug);
    logLevelComboBox_->addItem("Info", spdlog::level::info);
    logLevelComboBox_->addItem("Warning", spdlog::level::warn);
    logLevelComboBox_->addItem("Error", spdlog::level::err);
    logLevelComboBox_->addItem("Critical", spdlog::level::critical);

    logLevelComboBox_->setStyleSheet(
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
     "}");

       
    connect(logLevelComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogPanel::onLogLevelChanged);

    toolbarLayout->addWidget(logLevelComboBox_);
    


    searchInput_ = new QLineEdit();
    searchInput_->setPlaceholderText("搜索日志...");
    searchInput_->setStyleSheet(
        "QLineEdit {"
        "    background-color: white;"
        "    border: 1px solid #cccccc;"
        "    border-radius: 2px;"
        "    padding: 2px 5px;"
        "    max-width: 200px;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid #0078d7;"
        "}"
    );

    connect(searchInput_, &QLineEdit::textChanged, this, &LogPanel::onSearchTextChanged);

    toolbarLayout->addWidget(searchInput_);

    toolbarLayout->addStretch();

    clearButton_ = new QPushButton("清空");
    clearButton_->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    color: #666666;"
        "    padding: 2px 8px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e0e0e0;"
        "}"
    );

    connect(clearButton_, &QPushButton::clicked, this, &LogPanel::clearLog);

    toolbarLayout->addWidget(clearButton_);

    closeButton_ = new QPushButton("×");
    closeButton_->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    color: #666666;"
        "    padding: 2px 8px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e0e0e0;"
        "    color: #333333;"
        "}"
    );

    connect(closeButton_, &QPushButton::clicked, this, &LogPanel::closePanel);
    toolbarLayout->addWidget(closeButton_);

    mainLayout->addWidget(toolbar);

    // 日志文本区域
    logArea_ = new QTextEdit();
    logArea_->setReadOnly(true);
    logArea_->setStyleSheet(
        "QTextEdit {"
        "    background-color: white;"
        "    color: #333333;"
        "    border: none;"
        "    border-top: 1px solid #e0e0e0;"
        "    font-family: Consolas, 'Courier New', monospace;"
        "}"
    );
    mainLayout->addWidget(logArea_);
}
