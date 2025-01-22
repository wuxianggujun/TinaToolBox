#pragma once
#include <QObject>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <streambuf>
#include <mutex>
#include <queue>
#include "Singleton.hpp"

namespace TinaToolBox {
    class LogPanel;

    class StdoutRedirector : public std::streambuf {
    public:
        explicit StdoutRedirector(bool isStderr = false);

        ~StdoutRedirector() override;

    protected:
        int_type overflow(int_type c) override;
        int sync() override;

    private:
        void flushBuffer();
        std::string buffer_;
        bool isStderr_;
        std::streambuf *oldBuf_;
    };

    // 自定义的 spdlog sink
    template<typename Mutex>
    class LogPanelSink : public spdlog::sinks::base_sink<Mutex> {
    protected:
        void sink_it_(const spdlog::details::log_msg &msg) override;

        void flush_() override {
        }
    };

    // 添加日志条目结构体到公共区域
    struct LogEntry {
        QString text;
        spdlog::level::level_enum level;
        qint64 timestamp;
    };


    // 日志系统管理类
    class LogSystem : public QObject,public Singleton<LogSystem>{
        Q_OBJECT
        friend class Singleton<LogSystem>;
    public:
        
        void initialize() override;

        void shutdown() override;

        bool isInitialized() const;
        
        void log(const QString &message, spdlog::level::level_enum level);

        // 添加便捷方法
        template<typename... Args>
        void info(Args&&... args) {
            spdlog::info(std::forward<Args>(args)...);
        }

        template<typename... Args>
        void debug(Args&&... args) {
            spdlog::debug(std::forward<Args>(args)...);
        }

        template<typename... Args>
        void error(Args&&... args) {
            spdlog::error(std::forward<Args>(args)...);
        }

        void setLogLevel(spdlog::level::level_enum level);

        [[nodiscard]] QVector<LogEntry> getCachedLogs();
    signals:
        void logMessage(const QString &message, spdlog::level::level_enum level);

    private:
        LogSystem() :QObject(nullptr){}
        
        void setupQtMessageHandler();

        void setupSpdLogger();

        void setupStdRedirectors();

        void cacheLog(const LogEntry& entry);
        
        static constexpr size_t MAX_CACHED_LOGS = 1000;
        std::queue<LogEntry> cachedLogs_;
        std::unique_ptr<StdoutRedirector> stdoutRedirector_;
        std::unique_ptr<StdoutRedirector> stderrRedirector_;
        std::shared_ptr<LogPanelSink<std::mutex> > sink_;
        std::mutex mutex_;

        static void qtMessageHandler(QtMsgType type,
                                     const QMessageLogContext &context,
                                     const QString &msg);
    };
}
