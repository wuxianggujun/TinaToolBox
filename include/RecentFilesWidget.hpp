#pragma once

#include <QApplication>  // 用于访问剪贴板
#include <QClipboard>   // 用于复制到剪贴板
#include <QTreeWidget>
#include  <QMenu>
#include "FileHistory.hpp"

namespace TinaToolBox {
    class RecentFilesWidget : public QTreeWidget {
        Q_OBJECT

    public:
        explicit  RecentFilesWidget(QWidget *parent = nullptr);
        ~RecentFilesWidget() override;

        void loadRecentFiles();
        void addRecentFile(const QString &filePath);
        void removeRecentFile(const QString &filePath);

        void setShowScriptsOnly(bool showScriptOnly);

        void addFunctionEntry(const QString& functionName);
    signals:
        void fileSelected(const QString &filePath);
        void removeFileRequested(const QString &filePath);
        void functionEntryClicked(const QString &functionName);

    protected:
        void contextMenuEvent(QContextMenuEvent *event) override;
        bool event(QEvent *event) override;
        
    private:
        void setupUi();

        void setupConnections();

        void addFileItem(const FileHistory& fileHistory);

        void insertItem(QTreeWidgetItem* item,bool isFunctionEntry);
        
        void onItemDoubleClicked(QTreeWidgetItem *item, int column);

        void onItemClicked(QTreeWidgetItem* item, int column);
        
        QTreeWidgetItem* createFileItem(const FileHistory& fileHistory);
        void updateFileItem(QTreeWidgetItem* item, const FileHistory& fileHistory);
        [[nodiscard]] QString formatFileSize(qint64 size) const;

        void updateVisibleItems();
        bool isScriptFile(const QString & filePath) const;
        
        QMenu* contextMenu_{nullptr};
        FileHistoryManager& fileHistoryManager;
        bool showScriptsOnly_ = false;
    };

}
