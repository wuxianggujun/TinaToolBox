#include "RecentFilesWidget.hpp"

#include <QHeaderView>
#include<QContextMenuEvent>
#include <QFileInfo>
#include <QDateTime>
#include <spdlog/spdlog.h>

namespace TinaToolBox {
    RecentFilesWidget::RecentFilesWidget(QWidget *parent) : QTreeWidget(parent),
                                                            fileHistoryManager(FileHistoryManager::getInstance()) {
        setupUi();
        setupConnections();
        loadRecentFiles();
    }

    RecentFilesWidget::~RecentFilesWidget() = default;

    void RecentFilesWidget::loadRecentFiles() {
        clear();
        auto recentFiles = fileHistoryManager.getRecentFiles();
        for (const auto &fileHistory: recentFiles) {
            addTopLevelItem(createFileItem(fileHistory));
        }
    }

    void RecentFilesWidget::addRecentFile(const QString &filePath) {
        if (fileHistoryManager.addFileHistory(filePath)) {
            auto fileHistory = fileHistoryManager.getFileHistory(filePath);
            // 检查是否已存在
            for (int i = 0; i < topLevelItemCount(); ++i) {
                auto *item = topLevelItem(i);
                if (item->data(1, Qt::UserRole).toString() == filePath) {
                    updateFileItem(item, fileHistory);
                    return;
                }
            }
            // 添加新项
            insertTopLevelItem(0, createFileItem(fileHistory));
        }
    }

    void RecentFilesWidget::removeRecentFile(const QString &filePath) {
        if (fileHistoryManager.deleteFileHistory(filePath)) {
            for (int i = 0; i < topLevelItemCount(); ++i) {
                auto* item = topLevelItem(i);
                if (item->data(1, Qt::UserRole).toString() == filePath) {
                    delete takeTopLevelItem(i);
                    break;
                }
            }
        }
    }

    void RecentFilesWidget::contextMenuEvent(QContextMenuEvent *event) {
        if (itemAt(event->pos())) {
            contextMenu_->exec(event->globalPos());
        }
    }

    void RecentFilesWidget::setupUi() {
        setColumnCount(4);
        setHeaderLabels({"名称", "修改日期", "类型", "大小"});
        header()->setSectionResizeMode(QHeaderView::ResizeToContents);

        setAlternatingRowColors(true);
        setRootIsDecorated(false);
        setSortingEnabled(true);

        // 创建上下文
        contextMenu_ = new QMenu(this);
        contextMenu_->addAction("打开", [this]() {
            if (auto *item = currentItem()) {
                emit fileSelected(item->data(1, Qt::UserRole).toString());
            }
        });

        contextMenu_->addAction("从列表中移除", [this]() {
            if (auto *item = currentItem()) {
                QString filePath = item->data(1, Qt::UserRole).toString();
                emit removeFileRequested(filePath);
                removeRecentFile(filePath);
            }
        });
    }

    void RecentFilesWidget::setupConnections() {
        connect(this, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem *item, int column) {
            if (item) {
                emit fileSelected(item->data(1, Qt::UserRole).toString());
            }
        });
    }

    QTreeWidgetItem * RecentFilesWidget::createFileItem(const FileHistory &fileHistory) {
        auto* item = new QTreeWidgetItem();
        updateFileItem(item, fileHistory);
        return item;
    }

    void RecentFilesWidget::updateFileItem(QTreeWidgetItem *item, const FileHistory &fileHistory) {
        item->setText(0, fileHistory.fileName);
        item->setText(1, QFileInfo(fileHistory.filePath).path());
        item->setText(2, formatFileSize(fileHistory.fileSize));
        item->setText(3, fileHistory.modifiedDate.toString("yyyy-MM-dd HH:mm:ss"));
        item->setData(1, Qt::UserRole, fileHistory.filePath);
    }

    QString RecentFilesWidget::formatFileSize(qint64 size) const {
        const QStringList units = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double fileSize = size;

        while (fileSize >= 1024.0 && unitIndex < units.size() - 1) {
            fileSize /= 1024.0;
            unitIndex++;
        }

        return QString("%1 %2").arg(fileSize, 0, 'f', 2).arg(units[unitIndex]);
    }
}
