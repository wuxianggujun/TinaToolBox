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

#include "LogSystem.hpp"

namespace TinaToolBox {
    class LogPanel : public QWidget {
        Q_OBJECT

    struct LogEntryDisplay : public LogEntry {
            QColor color;
            explicit LogEntryDisplay(const LogEntry& entry):LogEntry(entry),color(Qt::black){}
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
        void highlightSearchText();
        
        [[nodiscard]] QColor getLevelColor(spdlog::level::level_enum level) const;
        
        QTextEdit *logArea_{};
        QLineEdit *searchInput_{};
        QPushButton *clearButton_{};
        QPushButton *closeButton_{};

        QComboBox *logLevelComboBox_{};
        
        QVector<LogEntryDisplay> displayEntries_;
        int currentLogLevel_;
        QString currentSearchText_;
    };
}
