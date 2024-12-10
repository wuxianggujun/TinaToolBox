//
// Created by wuxianggujun on 2024/11/23.
//

#include "LogPanel.hpp"

#include <iostream>
#include <QDateTime>
#include <QTextBlock>
#include <QScrollBar>
#include <QTextCursor>
#include <QTimer>
#include <spdlog/pattern_formatter.h>

#include "LogSystem.hpp"

namespace TinaToolBox {
    
    LogPanel::LogPanel(QWidget *parent) : QWidget(parent),currentLogLevel_(-1) {
        setupUI();

        // 连接到日志系统
        connect(&LogSystem::getInstance(), &LogSystem::logMessage,
                this, &LogPanel::onLogMessage,
                Qt::QueuedConnection);

        // 初始化日志系统
        LogSystem::getInstance().initialize(this);
    }

    LogPanel::~LogPanel() {
        LogSystem::getInstance().shutdown();
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
        logEntries_.clear(); // 同时清除存储的日志
        logArea_->clear();
    }

    void LogPanel::closePanel() {
        emit closed();
    }

    void LogPanel::onSearchTextChanged(const QString &text) {
        currentSearchText_ = text;
        filterLogs();
    }

    void LogPanel::onLogLevelChanged(int index) {
        currentLogLevel_ = logLevelComboBox_->currentData().toInt();
        filterLogs();
    }

    void LogPanel::onLogMessage(const QString &message, spdlog::level::level_enum level) {
        // 创建新的日志条目
        LogEntry entry{
            message,
            getLevelColor(level),
            level,
            QDateTime::currentMSecsSinceEpoch()
        };

        // 存储日志条目
        logEntries_.append(entry);

        // 应用过滤
        filterLogs();
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

    void LogPanel::filterLogs() {
        logArea_->clear();

        for (const auto &entry : logEntries_) {
            bool shouldShow = true;

            // 级别过滤
            if (currentLogLevel_ != -1 && entry.level != currentLogLevel_) {
                shouldShow = false;
            }

            // 搜索文本过滤
            if (!currentSearchText_.isEmpty() && 
                !entry.text.contains(currentSearchText_, Qt::CaseInsensitive)) {
                shouldShow = false;
                }

            if (shouldShow) {
                logArea_->setTextColor(entry.color);
                logArea_->append(entry.text);
            }
        }
    }

    QColor LogPanel::getLevelColor(spdlog::level::level_enum level) const {
        switch (level) {
            case spdlog::level::trace: return Qt::gray;
            case spdlog::level::debug: return Qt::darkGreen;
            case spdlog::level::info: return Qt::black;
            case spdlog::level::warn: return QColor(255, 165, 0);  // Orange
            case spdlog::level::err: return Qt::red;
            case spdlog::level::critical: return QColor(139, 0, 0);  // Dark red
            default: return Qt::black;
        }
    }

    QString LogPanel::getLevelName(spdlog::level::level_enum level) const {
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
}
