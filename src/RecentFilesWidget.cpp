#include "RecentFilesWidget.hpp"

#include <QHeaderView>
#include<QContextMenuEvent>
#include <QFileInfo>
#include <QDateTime>
#include <QToolTip>
#include <spdlog/spdlog.h>

namespace TinaToolBox {
    RecentFilesWidget::RecentFilesWidget(QWidget *parent) : QTreeWidget(parent),
                                                            fileHistoryManager(FileHistoryManager::getInstance()) {
        setupUi();
        setupConnections();
        loadRecentFiles();
    }

    RecentFilesWidget::~RecentFilesWidget() {
        if (contextMenu_) {
            delete contextMenu_;
            contextMenu_ = nullptr;
        }
        clear();
    };

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
                auto *item = topLevelItem(i);
                if (item->data(1, Qt::UserRole).toString() == filePath) {
                    delete takeTopLevelItem(i);
                    break;
                }
            }
        }
    }

    void RecentFilesWidget::contextMenuEvent(QContextMenuEvent *event) {
        // 确保右键点击的位置有项目
        if (itemAt(event->pos())) {
            contextMenu_->exec(event->globalPos());
        }
    }

    bool RecentFilesWidget::event(QEvent *event) {
        if (event->type() == QEvent::ToolTip) {
            auto *helpEvent = dynamic_cast<QHelpEvent *>(event);
        
            // 将全局坐标转换为视口坐标
            QPoint viewportPos = viewport()->mapFromGlobal(helpEvent->globalPos());
            QTreeWidgetItem *item = itemAt(viewportPos);
        
            if (item) {
                QString fullPath = item->data(0, Qt::UserRole).toString();
                spdlog::debug("Mouse at ({}, {}), Item: {}, Path: {}", 
                             viewportPos.x(), 
                             viewportPos.y(), 
                             item->text(0).toStdString(), 
                             fullPath.toStdString());
                         
                if (!fullPath.isEmpty()) {
                    QToolTip::showText(helpEvent->globalPos(), fullPath);
                } else {
                    QToolTip::hideText();
                }
                return true;
            }
            QToolTip::hideText();
        }
        return QTreeWidget::event(event);
    }

    void RecentFilesWidget::setupUi() {
        setColumnCount(2);
        setHeaderLabels({"名称", "大小"});
        // 设置列宽
        header()->setSectionResizeMode(0, QHeaderView::Stretch); // 名称列自动拉伸
        header()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // 大小列自适应内容

        setAlternatingRowColors(true);
        setRootIsDecorated(false);
        setSortingEnabled(true);
        setMouseTracking(true); // 启用鼠标追踪以支持悬停效果

        // 创建上下文
        contextMenu_ = new QMenu(this);
        contextMenu_->addAction("打开", [this]() {
            if (auto *item = currentItem()) {
                emit fileSelected(item->data(1, Qt::UserRole).toString());
            }
        });
            
        // 添加"复制路径"选项
        contextMenu_->addAction("复制路径", [this]() {
            if (auto *item = currentItem()) {
                QString filePath = item->data(0, Qt::UserRole).toString();
                QApplication::clipboard()->setText(filePath);
            }
        });
    
        // 添加分隔线
        contextMenu_->addSeparator();
        
        contextMenu_->addAction("从列表中移除", [this]() {
            if (auto *item = currentItem()) {
                QString filePath = item->data(1, Qt::UserRole).toString();
                emit removeFileRequested(filePath);
            }
        });
    }

    void RecentFilesWidget::setupConnections() {
        // 连接双击信号到文件打开槽
        connect(this, &QTreeWidget::itemDoubleClicked, 
                this, &RecentFilesWidget::onItemDoubleClicked);
        
    }

    void RecentFilesWidget::onItemDoubleClicked(QTreeWidgetItem *item, int column) {
        if (item) {
            // 获取存储的完整文件路径
            QString filePath = item->data(0, Qt::UserRole).toString();
            if (!filePath.isEmpty()) {
                emit fileSelected(filePath);
            }
        }
    }

    QTreeWidgetItem *RecentFilesWidget::createFileItem(const FileHistory &fileHistory) {
        QFileInfo fileInfo(fileHistory.filePath);
        auto *item = new QTreeWidgetItem();

        // 设置文件名
        item->setText(0, fileInfo.fileName());

        // 设置文件大小
        item->setText(1, formatFileSize(fileInfo.size()));

        // 存储完整路径用于后续操作
        item->setData(0, Qt::UserRole, fileHistory.filePath);
        return item;
    }

    void RecentFilesWidget::updateFileItem(QTreeWidgetItem *item, const FileHistory &fileHistory) {
        QFileInfo fileInfo(fileHistory.filePath);
        item->setText(0, fileInfo.fileName());
        item->setText(1, formatFileSize(fileInfo.size()));
        item->setData(0, Qt::UserRole, fileHistory.filePath);
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
