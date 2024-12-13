// MainWindowMenuBar.cpp
#include "MainWindowMenuBar.hpp"
#include <QTimer>
#include <QEvent>
#include <QStyle>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>
#include <spdlog/spdlog.h>

namespace TinaToolBox {
    MainWindowMenuBar::MainWindowMenuBar(QWidget *parent) : QWidget(parent) {
        setFixedHeight(MENU_BUTTON_HEIGHT);
        setMouseTracking(true);

        initializeMenus();
        initializeControlButtons();
    }

    void MainWindowMenuBar::updateMaximizeButton(bool isMaximized) {
        isMaximized_ = isMaximized;
        for (auto &button: controlButtons_) {
            if (button.type == "max") {
                button.icon = isMaximized ? "❐" : "□";
                update();
                break;
            }
        }
    }

    void MainWindowMenuBar::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 绘制每个菜单项
        for (const auto &item: menuItems_) {
            // 根据活动状态选择背景色
            QColor bgColor = backgroundColor;
            if (item.isActive) {
                bgColor = activeColor; // 使用选中背景色
            }else if (item.isHovered) {
                bgColor = hoverColor;  // 使用悬停颜色
            }

            painter.fillRect(item.rect, bgColor);
            painter.setPen(textColor);
            painter.drawText(item.rect, Qt::AlignCenter, item.text);
        }

        // 绘制控制按钮
        for (const auto &button: controlButtons_) {
            drawControlButton(painter, button);
        }
    }

    void MainWindowMenuBar::mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            if (MenuItem *item = getMenuItemAt(event->pos())) {
                if (!item) {
                    hideActiveMenu();
                    return;
                }

                if (activeMenuItem_ == item) {
                    hideActiveMenu();
                } else {
                    showMenuAt(item);
                }
            } else if (ControlButton *button = getControlButtonAt(event->pos())) {
                if (button->type == "min") {
                    emit minimizeClicked();
                } else if (button->type == "max") {
                    emit maximizeClicked();
                } else if (button->type == "close") {
                    emit closeClicked();
                } else if (button->type == "settings") {
                    emit settingsClicked();
                }
            }
        }
    }

    void MainWindowMenuBar::mouseMoveEvent(QMouseEvent *event) {
        bool needsUpdate = false;

        // 更新菜单项悬停状态
        for (auto &item: menuItems_) {
            bool wasHovered = item.isHovered;
            item.isHovered = item.rect.contains(event->pos());
            if (wasHovered != item.isHovered) {
                needsUpdate = true;
            }
        }

        // 更新控制按钮悬停状态
        for (auto &button: controlButtons_) {
            bool wasHovered = button.isHovered;
            button.isHovered = button.rect.contains(event->pos());
            if (wasHovered != button.isHovered) {
                needsUpdate = true;
            }
        }

        if (needsUpdate) {
            update();
        }
    }

    void MainWindowMenuBar::mouseReleaseEvent(QMouseEvent *event) {
        // 处理鼠标释放事件
        if (event->button() == Qt::LeftButton) {
            // 如果点击在菜单项外并且没有活动菜单，清除所有活动状态
            if (!getMenuItemAt(event->pos()) && !activeMenu_) {
                for (auto &item: menuItems_) {
                    if (item.isActive) {
                        item.isActive = false;
                        update();
                    }
                }
            }
        }
        QWidget::mouseReleaseEvent(event);
    }

    void MainWindowMenuBar::leaveEvent(QEvent *event) {
        bool needsUpdate = false;
        // 清除所有悬停状态
        for (auto &item: menuItems_) {
            if (item.isHovered) {
                item.isHovered = false;
                needsUpdate = true;
            }
        }

        for (auto &button: controlButtons_) {
            if (button.isHovered) {
                button.isHovered = false;
                needsUpdate = true;
            }
        }

        if (needsUpdate) {
            update();
        }

        QWidget::leaveEvent(event);
    }

    bool MainWindowMenuBar::eventFilter(QObject *watched, QEvent *event) {
        if (auto *menu = qobject_cast<QMenu *>(watched)) {
            if (event->type() == QEvent::Hide) {
                
                QPoint localPos = mapFromGlobal(QCursor::pos());

                // 检查是否在切换到其他菜单项
                MenuItem *newItem = getMenuItemAt(localPos);

                if (newItem && newItem != activeMenuItem_ && newItem->rect.contains(localPos)) {
                    spdlog::debug("Switching menus to: {}", newItem->text.toStdString());
                    QTimer::singleShot(0, this, [this, newItem]() {
                        showMenuAt(newItem);
                    });
                } else if (!newItem || !newItem->rect.contains(localPos)) {
                    spdlog::debug("Not switching menus, clearing state");
                    hideActiveMenu();
                }
                return true;
            }
        }

        // 当有活动菜单时，检查鼠标移动
        if (activeMenu_ && event->type() == QEvent::MouseMove) {
            QPoint globalPos = QCursor::pos();
            QPoint localPos = mapFromGlobal(globalPos);

            MenuItem *newItem = getMenuItemAt(localPos);
            if (newItem && newItem != activeMenuItem_ && !newItem->isActive) {
                spdlog::debug("Active menu exists and mouse moved to new item");
                showMenuAt(newItem);
                return true;
            }
        }

        return QWidget::eventFilter(watched, event);
    }

    void MainWindowMenuBar::resizeEvent(QResizeEvent *event) {
        QWidget::resizeEvent(event);
        updateLayout();
    }


    void MainWindowMenuBar::initializeMenus() {
        struct MenuData {
            QString text;
            QList<QPair<QString, QString> > actions;
        };


        QList<MenuData> menuData = {
            {
                "文件(&F)", {
                    {"新建", "Ctrl+N"},
                    {"打开", "Ctrl+O"},
                    {"保存", "Ctrl+S"},
                    {"另存为", "Ctrl+Shift+S"},
                    {"", ""}, // separator
                    {"导入", "Ctrl+I"},
                    {"导出", "Ctrl+E"},
                    {"", ""}, // separator
                    {"退出", "Alt+F4"}
                }
            },
            {
                "编辑(&E)", {
                    {"撤销", "Ctrl+Z"},
                    {"重做", "Ctrl+Y"},
                    {"", ""}, // separator
                    {"剪切", "Ctrl+X"},
                    {"复制", "Ctrl+C"},
                    {"粘贴", "Ctrl+V"}
                }
            }
        };

        for (const auto &data: menuData) {
            MenuItem item;
            item.text = data.text;
            item.menu = new QMenu(this);
            item.menu->installEventFilter(this);

            for (const auto &[action, shortcut]: data.actions) {
                if (action.isEmpty()) {
                    item.menu->addSeparator();
                } else {
                    QAction *menuAction = item.menu->addAction(action);
                    if (!shortcut.isEmpty()) {
                        menuAction->setShortcut(QKeySequence(shortcut));
                    }

                    connect(menuAction, &QAction::triggered, this, [this,action]() {
                        emit menuActionTriggered(action);
                    });
                }
            }
            menuItems_.append(item);
        }
    }

    void MainWindowMenuBar::initializeControlButtons() {
        controlButtons_ = {
            {"⚙", QRect(), false, "settings"},
            {"─", QRect(), false, "min"},
            {"□", QRect(), false, "max"},
            {"×", QRect(), false, "close"}
        };
    }

    void MainWindowMenuBar::drawMenuItem(QPainter &painter, const MenuItem &item) {
        QColor bgColor = backgroundColor;
        if (item.isActive) {
            bgColor = activeColor;
        } else if (item.isHovered) {
            bgColor = hoverColor;
        }
        painter.fillRect(item.rect, bgColor);
        painter.setPen(textColor);
        painter.drawText(item.rect, Qt::AlignCenter, item.text);
    }

    void MainWindowMenuBar::drawControlButton(QPainter &painter, const ControlButton &button) {
        QColor bgColor = backgroundColor;
        QColor iconColor = textColor;

        if (button.isHovered) {
            if (button.type == "close") {
                bgColor = closeHoverColor;
                iconColor = Qt::white;
            } else {
                bgColor = hoverColor;
            }
        }

        painter.fillRect(button.rect, bgColor);
        painter.setPen(iconColor);
        painter.drawText(button.rect, Qt::AlignCenter, button.icon);
    }

    MainWindowMenuBar::MenuItem *MainWindowMenuBar::getMenuItemAt(const QPoint &pos) {
        for (auto &item: menuItems_) {
            if (item.rect.contains(pos)) {
                return &item;
            }
        }
        return nullptr;
    }

    MainWindowMenuBar::ControlButton *MainWindowMenuBar::getControlButtonAt(const QPoint &pos) {
        for (auto &button: controlButtons_) {
            if (button.rect.contains(pos)) {
                return &button;
            }
        }
        return nullptr;
    }

    void MainWindowMenuBar::showMenuAt(MenuItem *item) {
        if (!item || item == activeMenuItem_) {
            return;
        }
        spdlog::debug("showMenuAt called for menu item: {}", item->text.toStdString());

        // 更新状态
        if (activeMenuItem_) {
            activeMenuItem_->isActive = false;
        }

        activeMenuItem_ = item;
        activeMenuItem_->isActive = true;

        // 如果有活动菜单，先隐藏
        if (activeMenu_) {
            activeMenu_->hide();
        }

        activeMenu_ = item->menu;

        QPoint pos = mapToGlobal(QPoint(item->rect.x(), item->rect.bottom()));
        spdlog::debug("Showing menu at global pos: ({}, {})", pos.x(), pos.y());
        
        activeMenu_->popup(pos);
        
        update();

        spdlog::debug("After showMenuAt - activeMenu_={}, activeMenuItem_={}",
                      activeMenu_ ? "true" : "false",
                      activeMenuItem_ ? activeMenuItem_->text.toStdString() : "null");
    }

    void MainWindowMenuBar::hideActiveMenu() {
        spdlog::debug("Hiding active menu");
        if (activeMenuItem_) {
            activeMenuItem_->isActive = false;
        }
        activeMenuItem_ = nullptr;
        activeMenu_ = nullptr;
        update();
    }

    void MainWindowMenuBar::updateLayout() {
        int x = 0;

        // 更新菜单项位置
        for (auto &item: menuItems_) {
            QFontMetrics fm(font());
            int width = fm.horizontalAdvance(item.text) + 2 * MENU_BUTTON_PADDING;
            item.rect = QRect(x, 0, width, MENU_BUTTON_HEIGHT);
            x += width;
        }

        // 更新控制按钮位置
        x = width() - CONTROL_BUTTON_WIDTH * controlButtons_.size();
        for (auto &button: controlButtons_) {
            button.rect = QRect(x, 0, CONTROL_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
            x += CONTROL_BUTTON_WIDTH;
        }
    }
}
