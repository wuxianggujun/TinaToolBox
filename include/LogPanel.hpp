//
// Created by wuxianggujun on 2024/11/23.
//

#pragma once

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

namespace TinaToolBox {
    class LogPanel : public QWidget {
        Q_OBJECT

    private:
        struct LogEntry {
            QString text;
            QColor color;
            spdlog::level::level_enum level;
            qint64 timestamp; // 添加时间戳便于排序和过滤
        };
    
    public:
        explicit LogPanel(QWidget *parent = nullptr);

        ~LogPanel() override;
    private slots:
        
        void clearLog();
        void closePanel();
        void onSearchTextChanged(const QString &text);
        void onLogLevelChanged(int index);
        void onLogMessage(const QString &message, spdlog::level::level_enum level);

    signals:
        void closed();
    
    private:
        void setupUI();
        void filterLogs();
        
        [[nodiscard]] QColor getLevelColor(spdlog::level::level_enum level) const;
        [[nodiscard]] QString getLevelName(spdlog::level::level_enum level) const;
        
        QTextEdit *logArea_{};
        QLineEdit *searchInput_{};
        QPushButton *clearButton_{};
        QPushButton *closeButton_{};

        QComboBox *logLevelComboBox_{};
        
        QVector<LogEntry> logEntries_;
        int currentLogLevel_;
        QString currentSearchText_;
    };
}
