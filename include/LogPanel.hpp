//
// Created by wuxianggujun on 2024/11/23.
//

#ifndef TINA_TOOL_BOX_LOG_PANEL_HPP
#define TINA_TOOL_BOX_LOG_PANEL_HPP

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <qcombobox.h>

class LogPanel;

// 创建一个自定义的spdlog sink
template<typename Mutex>
class LogPanelSink : public spdlog::sinks::base_sink<Mutex> {
public:
    explicit LogPanelSink(LogPanel* panel) : panel_(panel) {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override {}

private:
    LogPanel* panel_;
};

class LogPanel : public QWidget {

Q_OBJECT

private:
    struct LogEntry {
        QString text;
        QColor color;
        spdlog::level::level_enum level;
    };

public:
    explicit LogPanel(QWidget* parent = nullptr);
    ~LogPanel() override;

    void appendLog(const QString& text);
    void appendLogWithColor(const QString& text, const QColor& color);

signals:
    void closed();
    void logMessage(const QString& message, const QColor& color);

public slots:
    void clearLog();
    void closePanel();
    void onSearchTextChanged(const QString& text);

    void onLogLevelChanged(int index);

private:
    void setupUI();
    void setupLogHandlers();

    QTextEdit* logArea_;
    QLineEdit* searchInput_;
    QPushButton* clearButton_;
    QPushButton* closeButton_;

    QComboBox* logLevelComboBox_;
    void filterLogsByLevel();
    int currentLogLevel_;

    QVector<LogEntry> logEntries_;
    std::shared_ptr<LogPanelSink<std::mutex>> sink_;
    static void qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    static LogPanel* instance_;

    static std::streambuf* oldCoutBuf;
    static std::streambuf* oldCerrBuf;
    
};

#endif //TINA_TOOL_BOX_LOG_PANEL_HPP
