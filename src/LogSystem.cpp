#include "LogSystem.hpp"
#include "LogPanel.hpp"
#include <QDateTime>
#include <iostream>

namespace TinaToolBox {
    StdoutRedirector::StdoutRedirector(bool isStderr): isStderr_(isStderr) {
        oldBuf_ = isStderr_ ? std::cerr.rdbuf() : std::cout.rdbuf();
        if (isStderr_) {
            std::cerr.rdbuf(this);
        } else {
            std::cout.rdbuf(this);
        }
    }

    StdoutRedirector::~StdoutRedirector() {
        if (isStderr_) {
            std::cerr.rdbuf(oldBuf_);
        } else {
            std::cout.rdbuf(oldBuf_);
        }
    }

    std::streambuf::int_type StdoutRedirector::overflow(int_type c) {
        if (c != EOF) {
            if (c == '\n') {
                auto &logSystem = LogSystem::getInstance();
                logSystem.log(QString::fromStdString(buffer_), isStderr_ ? spdlog::level::err : spdlog::level::info);
                buffer_.clear();
            } else {
                buffer_ += static_cast<char>(c);
            }
        }
        return c;
    }

    
    template<typename Mutex>
    void LogPanelSink<Mutex>::sink_it_(const spdlog::details::log_msg &msg) {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        QString text = QString::fromUtf8(formatted.data(), static_cast<int>(formatted.size()));
        LogSystem::getInstance().log(text, msg.level);
    }
    
    void LogSystem::setupQtMessageHandler() {
        qInstallMessageHandler(qtMessageHandler);
    }

    void LogSystem::setupSpdLogger() {
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        sink_ = std::make_shared<LogPanelSink<std::mutex> >();
        auto logger = spdlog::default_logger();
        logger->sinks().clear();
        logger->sinks().push_back(sink_);
        // 确保设置足够低的日志级别以捕获所有日志
        logger->set_level(spdlog::level::trace);
        spdlog::set_level(spdlog::level::trace);
    }

    void LogSystem::setupStdRedirectors() {
        stdoutRedirector_ = std::make_unique<StdoutRedirector>(false);
        stderrRedirector_ = std::make_unique<StdoutRedirector>(false);
    }

    void LogSystem::cacheLog(const LogEntry &entry) {
        cachedLogs_.push(entry);
        while (cachedLogs_.size()>MAX_CACHED_LOGS) {
            cachedLogs_.pop();
        }
    }

    void LogSystem::qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        auto &logSystem = getInstance();
        QString fullMsg = msg;

        if (context.file) {
            fullMsg += QString(" (%1:%2)").arg(context.file).arg(context.line);
        }

        spdlog::level::level_enum level;
        switch (type) {
            case QtDebugMsg: level = spdlog::level::debug;
                break;
            case QtInfoMsg: level = spdlog::level::info;
                break;
            case QtWarningMsg: level = spdlog::level::warn;
                break;
            case QtCriticalMsg: level = spdlog::level::err;
                break;
            case QtFatalMsg: level = spdlog::level::critical;
                break;
            default: level = spdlog::level::info;
        }

        logSystem.log(fullMsg, level);
    }
    

    void LogSystem::initialize() {
        std::lock_guard<std::mutex> lock(mutex_);
        setupSpdLogger();
        setupQtMessageHandler();
        setupStdRedirectors();
    }

    void LogSystem::shutdown() {
        std::lock_guard lock(mutex_);
        stdoutRedirector_.reset();
        stderrRedirector_.reset();
        sink_.reset();
    }

    void LogSystem::log(const QString &message, spdlog::level::level_enum level) {
        std::lock_guard lock(mutex_);
        LogEntry entry{
            message,
            level,
            QDateTime::currentMSecsSinceEpoch()
        };

        cacheLog(entry);

        emit logMessage(message, level);
    }

    void LogSystem::setLogLevel(spdlog::level::level_enum level) {
        spdlog::set_level(level);
    }

    QVector<LogEntry> LogSystem::getCachedLogs() {
        std::lock_guard<std::mutex> lock(mutex_);
        QVector<LogEntry> logs;
        auto tempQueue = cachedLogs_;  // 创建副本
        while (!tempQueue.empty()) {
            logs.append(tempQueue.front());
            tempQueue.pop();
        }
        return logs;
    }
}
