#pragma once
#include <QObject>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <streambuf>
#include <mutex>
#include <queue>

namespace TinaToolBox {
    class LogPanel;

    class StdoutRedirector : public std::streambuf {
    public:
        explicit StdoutRedirector(bool isStderr = false);

        ~StdoutRedirector() override;

    protected:
        int_type overflow(int_type c) override;

    private:
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
    class LogSystem : public QObject {
        
        Q_OBJECT

    public:
        static LogSystem &getInstance();

        void initialize();

        void shutdown();

        void log(const QString &message, spdlog::level::level_enum level);

        void setLogLevel(spdlog::level::level_enum level);

        [[nodiscard]] QVector<LogEntry> getCachedLogs();
    signals:
        void logMessage(const QString &message, spdlog::level::level_enum level);

    private:
        LogSystem();

        ~LogSystem() override;

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
