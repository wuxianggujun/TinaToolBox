#pragma once

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
    signals:
        void fileSelected(const QString &filePath);
        void removeFileRequested(const QString &filePath);

    protected:
        void contextMenuEvent(QContextMenuEvent *event) override;
        
    private:
        void setupUi();

        void setupConnections();

        QTreeWidgetItem* createFileItem(const FileHistory& fileHistory);
        void updateFileItem(QTreeWidgetItem* item, const FileHistory& fileHistory);
        [[nodiscard]] QString formatFileSize(qint64 size) const;
        
        QMenu* contextMenu_{};
        FileHistoryManager& fileHistoryManager;
    };
}
