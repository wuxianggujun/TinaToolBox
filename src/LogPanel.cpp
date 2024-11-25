//
// Created by wuxianggujun on 2024/11/23.
//

#include "LogPanel.hpp"
#include <QScrollBar>
#include <QTextCursor>

LogPanel* LogPanel::instance_ = nullptr;

template<typename Mutex>
void LogPanelSink<Mutex>::sink_it_(const spdlog::details::log_msg& msg) {
    if (!panel_) return;

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
            color = QColor(255, 165, 0);  // Orange
            break;
        case spdlog::level::err:
            color = Qt::red;
            break;
        case spdlog::level::critical:
            color = QColor(139, 0, 0);  // Dark red
            break;
        default:
            color = Qt::black;
    }

    // 使用 Qt::QueuedConnection 确保在主线程中更新 UI
    QMetaObject::invokeMethod(panel_, "logMessage", Qt::QueuedConnection,
                            Q_ARG(QString, text), Q_ARG(QColor, color));
}

void LogPanel::qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    if (!instance_) return;

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

    QString logMessage = prefix + msg;
    if (context.file) {
        logMessage += QString(" (%1:%2)").arg(context.file).arg(context.line);
    }

    QMetaObject::invokeMethod(instance_, "logMessage", Qt::QueuedConnection,
                            Q_ARG(QString, logMessage), Q_ARG(QColor, color));
}

LogPanel::LogPanel(QWidget* parent) : QWidget(parent) {
    instance_ = this;
    setupUI();
    setupLogHandlers();
    
    // 连接信号和槽
    connect(this, &LogPanel::logMessage, this, [this](const QString& message, const QColor& color) {
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
    sink_ = std::make_shared<LogPanelSink<std::mutex>>(this);
    auto logger = spdlog::default_logger();
    logger->sinks().push_back(sink_);
    
    // 设置Qt消息处理器
    qInstallMessageHandler(qtMessageHandler);
}

void LogPanel::appendLog(const QString& text) {
    logArea_->append(text);
}

void LogPanel::appendLogWithColor(const QString& text, const QColor& color) {
    logArea_->setTextColor(color);
    logArea_->append(text);
}

void LogPanel::clearLog() {
    logArea_->clear();
}

void LogPanel::closePanel() {
    emit closed();
}

void LogPanel::onSearchTextChanged(const QString& text) {
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
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* toolbar = new QWidget();
    toolbar->setStyleSheet(
        "QWidget {"
        "    background-color: #f5f5f5;"
        "    border-top: 1px solid #e0e0e0;"
        "}"
    );

    auto* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(5, 2, 5, 2);

    auto* outputLabel = new QLabel("输出");
    outputLabel->setStyleSheet("color: #333333; font-weight: bold;");
    toolbarLayout->addWidget(outputLabel);

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
