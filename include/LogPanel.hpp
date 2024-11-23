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
#include <QScrollBar>
#include <QTextCursor>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

template<class Mutex>
class qt_sink : public spdlog::sinks::base_sink<Mutex> {
public:
    explicit qt_sink(QTextEdit *text_edit): text_edit_(text_edit) {
    }

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override {
        if (!text_edit_) return;

        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        QString text = QString::fromUtf8(formatted.data(), formatted.size());

        // 使用 Qt::QueuedConnection 确保在主线程中更新 UI
        QMetaObject::invokeMethod(text_edit_, "append", Qt::QueuedConnection, Q_ARG(QString, text));
    }

    void flush_() override {
    }

private:
    QTextEdit *text_edit_;
};

using qt_sink_mt = qt_sink<std::mutex>;
using qt_sink_st = qt_sink<spdlog::details::null_mutex>;

class LogPanel : public QWidget{

Q_OBJECT
public:
    explicit LogPanel(QWidget* parent = nullptr);
    ~LogPanel() override;

    void cleanup();
    void appendLog(const QString& text);

    signals:
    void searchTextChanged(const QString& text);
    void closed();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onSearchTextChanged(const QString& text);
    void highlightSearchText(const QString& text);
    void clearLog();
    void closePanel();
    
private:
    void setupUI();
    void setupLogger();

    QTextEdit* logArea_;
    QLineEdit* searchInput_;
    QPushButton* clearButton_;
    QPushButton* closeButton_;

    std::shared_ptr<spdlog::logger> logger_;
    std::shared_ptr<qt_sink_mt> qt_sink_mt_;
};



#endif //TINA_TOOL_BOX_LOG_PANEL_HPP
