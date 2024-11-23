//
// Created by wuxianggujun on 2024/11/23.
//

#include "LogPanel.hpp"

LogPanel::LogPanel(QWidget *parent) : QWidget(parent) {
    setupUI();
    setupLogger();
}

LogPanel::~LogPanel() {
    cleanup();
}

void LogPanel::cleanup() {
    if (logger_) {
        // 清除所有日志记录器
        spdlog::drop_all();
        logger_.reset();
    }
    qt_sink_mt_.reset();
}

void LogPanel::appendLog(const QString &text) {
    logArea_->append(text);
}

void LogPanel::closeEvent(QCloseEvent *event) {
    cleanup();
    QWidget::closeEvent(event);
}

void LogPanel::onSearchTextChanged(const QString &text) {
    emit searchTextChanged(text);
    highlightSearchText(text);
}

void LogPanel::highlightSearchText(const QString &text) {
    if (text.isEmpty()) {
        // 清除所有高亮
        QTextCursor cursor = logArea_->textCursor();
        cursor.select(QTextCursor::Document);
        QTextCharFormat format;
        format.setBackground(Qt::white);
        cursor.mergeCharFormat(format);
        return;
    }

    // 保存滚动位置
    QScrollBar *scrollbar = logArea_->verticalScrollBar();
    int scrollPos = scrollbar->value();

    // 高亮搜索文本
    QTextCursor cursor = logArea_->textCursor();
    cursor.movePosition(QTextCursor::Start);

    // 清除之前的高亮
    cursor.select(QTextCursor::Document);
    QTextCharFormat format;
    format.setBackground(Qt::white);
    cursor.mergeCharFormat(format);
    cursor.clearSelection();

    // 高亮新的匹配项
    QTextCharFormat highlightFormat;
    highlightFormat.setBackground(QColor("#cce8ff"));

    cursor.movePosition(QTextCursor::Start);
    while (true) {
        cursor = logArea_->document()->find(text, cursor);
        if (cursor.isNull()) {
            break;
        }
        cursor.mergeCharFormat(highlightFormat);
    }

    // 恢复滚动位置
    scrollbar->setValue(scrollPos);
}

void LogPanel::clearLog() {
    logArea_->clear();
}

void LogPanel::closePanel() {
    hide();
    emit closed();
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

    // 关闭按钮
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

    connect(closeButton_, &QPushButton::clicked,
            this, &LogPanel::closePanel);
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

void LogPanel::setupLogger() {
    // 创建自定义 sink
    qt_sink_mt_ = std::make_shared<qt_sink_mt>(logArea_);

    // 创建日志记录器
    logger_ = std::make_shared<spdlog::logger>("qt_logger", qt_sink_mt_);

    // 设置日志格式
    logger_->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^%l%$] %v");

    // 设置为默认日志记录器
    spdlog::set_default_logger(logger_);

    // 设置日志级别
    spdlog::set_level(spdlog::level::info);
}
