// MainWindowMenuBar.cpp
#include "MainWindowMenuBar.hpp"
#include <QTimer>
#include <QEvent>
#include <QStyle>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>
#include <spdlog/spdlog.h>

#include "UIConfig.hpp"

namespace TinaToolBox {
    MainWindowMenuBar::MainWindowMenuBar(QWidget *parent) : QWidget(parent) {
        setFixedHeight(MENU_HEIGHT);
        setMouseTracking(true);
        setAttribute(Qt::WA_TranslucentBackground); // 添加这行

        initializeMenus();
        initializeControlButtons();
    }

    void MainWindowMenuBar::updateMaximizeButton(bool isMaximized) {
        isMaximized_ = isMaximized;
        for (auto &button: controlButtons_) {
            if (button.name == "maximize") {
                // 根据窗口状态切换图标
                if (window()->isMaximized()) {
                    button.icon = QIcon(":/icons/chrome-restore.svg");
                } else {
                    button.icon = QIcon(":/icons/chrome-maximize.svg");
                }
                update();
                break;
            }
        }
    }

    bool MainWindowMenuBar::isInDraggableArea(const QPoint &pos) const {
        // 检查是否在菜单项区域外
        for (const auto &item: menuItems_) {
            if (item.rect.contains(pos)) {
                return false;
            }
        }
        // 检查是否在控制按钮区域外
        for (const auto &button: controlButtons_) {
            if (button.rect.contains(pos)) {
                return false;
            }
        }
        return true;
    }

    void MainWindowMenuBar::paintEvent(QPaintEvent *event) {
        Q_UNUSED(event);
        
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 绘制带圆角的背景
        painter.setPen(Qt::NoPen);
        painter.setBrush(backgroundColor);

        // 获取圆角半径
        int radius = UIConfig::getInstance().cornerRadius();
        // 只绘制上半部分的圆角
        QRect rect = this->rect();
        QPainterPath path;

        if (window()->isMaximized()) {
            // 窗口最大化时，绘制普通矩形
            path.addRect(rect);
        } else {
            // 非最大化时，绘制带圆角的路径
            path.moveTo(0, rect.height());
            path.lineTo(0, radius);
            path.arcTo(0, 0, radius*2, radius*2, 180, -90);
            path.lineTo(rect.width() - radius, 0);
            path.arcTo(rect.width() - radius*2, 0, radius*2, radius*2, 90, -90);
            path.lineTo(rect.width(), rect.height());
            path.lineTo(0, rect.height());
        }

        painter.drawPath(path);

        // 绘制菜单项
        for (const auto &item: menuItems_) {
            QColor bgColor = backgroundColor;
            if (item.isActive) {
                bgColor = activeColor;
            } else if (item.isHovered) {
                bgColor = hoverColor;
            }

            // 创建菜单项的路径
            QPainterPath itemPath;
            if (item.rect.x() == 0 && !window()->isMaximized()) {
                // 如果是第一个菜单项 左侧需要圆角
                itemPath.moveTo(item.rect.x(), item.rect.bottom());
                itemPath.lineTo(item.rect.x(), item.rect.top() + radius);
                itemPath.arcTo(item.rect.x(), item.rect.top(), radius * 2, radius * 2, 180, -90);
                itemPath.lineTo(item.rect.right(), item.rect.top());
                itemPath.lineTo(item.rect.right(), item.rect.bottom());
            } else {
                // 普通矩形
                itemPath.addRect(item.rect);
            }

            // 填充背景
            painter.fillPath(itemPath, bgColor);
            painter.setPen(textColor);
            painter.drawText(item.rect, Qt::AlignCenter, item.text);
        }

        // 绘制控制按钮
        for (const auto &button: controlButtons_) {
            drawControlButton(painter, button);
        }

        // 绘制底部线条
        painter.setPen(lineColor);
        painter.drawLine(0, rect.height() - 1, rect.width(), rect.height() - 1);
    }

    void MainWindowMenuBar::mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            if (MenuItem *item = getMenuItemAt(event->pos())) {
                if (activeMenuItem_ == item) {
                    hideActiveMenu();
                } else {
                    showMenuAt(item);
                }
            } else if (ControlButton *button = getControlButtonAt(event->pos())) {
                // 在发送信号之前先检查按钮是否有效
                if (!button) return;

                if (button->name == "minimize") {
                    emit minimizeClicked();
                } else if (button->name == "maximize") {
                    emit maximizeClicked();
                } else if (button->name == "close") {
                    emit closeClicked();
                } else if (button->name == "settings") {
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
                "文件(F)", {
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
                "编辑(E)", {
                    {"撤销", "Ctrl+Z"},
                    {"重做", "Ctrl+Y"},
                    {"", ""}, // separator
                    {"剪切", "Ctrl+X"},
                    {"复制", "Ctrl+C"},
                    {"粘贴", "Ctrl+V"}
                }
            }
        };

        int x = 0;
        for (const auto &data: menuData) {
            MenuItem item;
            item.text = data.text;
            item.menu = new QMenu(this);
            item.rect = QRect(x, 0, MENU_ITEM_WIDTH, MENU_HEIGHT);
            item.menu->installEventFilter(this);
            x += MENU_ITEM_WIDTH;

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
        // 按照minimize、maximize、close的顺序初始化
        controlButtons_ = {
            {
                "settings", QRect(0, 0, CONTROL_BUTTON_WIDTH, MENU_HEIGHT), false,
                QIcon(":/icons/settings-gear.svg"), QSize(ICON_SIZE, ICON_SIZE), true
            },
            {
                "minimize", QRect(0, 0, CONTROL_BUTTON_WIDTH, MENU_HEIGHT), false,
                QIcon(":/icons/chrome-minimize.svg"), QSize(ICON_SIZE, ICON_SIZE), false
            },
            {
                "maximize", QRect(0, 0, CONTROL_BUTTON_WIDTH, MENU_HEIGHT), false,
                QIcon(":/icons/chrome-restore.svg"), QSize(ICON_SIZE, ICON_SIZE), false
            },
            {
                "close", QRect(0, 0, CONTROL_BUTTON_WIDTH, MENU_HEIGHT), false,
                QIcon(":/icons/chrome-close.svg"), QSize(ICON_SIZE, ICON_SIZE), false
            }
        };
        updateControlButtonsPosition();
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
        
        // 获取圆角半径
        int radius = UIConfig::getInstance().cornerRadius();
   
        // 检查是否是最右边的按钮（关闭按钮）
        bool isCloseButton = (button.name == "close");

        if (button.useShapeDetection) {
            // 设置按钮只在图标区域绘制背景
            if (button.isHovered) {
                int x = button.rect.x() + (button.rect.width() - button.iconSize.width()) / 2;
                int y = button.rect.y() + (button.rect.height() - button.iconSize.height()) / 2;
                QRect iconRect(x, y, button.iconSize.width(), button.iconSize.height());
                painter.fillRect(iconRect, hoverColor);
            }
        } else {
            // 获取背景颜色
            QColor bgColor = button.isHovered
                ? (isCloseButton ? closeHoverColor : hoverColor)
                : backgroundColor;

            if (isCloseButton) {
                // 为关闭按钮创建带圆角的路径
                QPainterPath path;
                QRect r = button.rect;
            
                // 确保右边界延伸到窗口边缘
                r.setRight(width());  // 将按钮的右边界设置为窗口宽度
                
                if (window()->isMaximized()) {
                    // 窗口最大化时，绘制普通矩形
                    path.addRect(r);
                } else {
                    // 非最大化时，绘制带圆角的路径
                    path.moveTo(r.left(), r.bottom());  // 左下角
                    path.lineTo(r.left(), r.top());     // 左边线
                    path.lineTo(r.right() - radius, r.top()); // 使用配置的圆角半径
                    path.arcTo(r.right() - (radius * 2), r.top(), radius * 2, radius * 2, 90, -90); // 右上角圆弧
                    path.lineTo(r.right(), r.bottom());  // 右边线
                    path.lineTo(r.left(), r.bottom());   // 底边线
                }
                
                painter.setPen(Qt::NoPen);
                painter.setBrush(bgColor);
                painter.drawPath(path);
            } else {
                // 其他按钮保持矩形背景
                painter.fillRect(button.rect, bgColor);
            }
        }

        // 绘制图标
        if (!button.icon.isNull()) {
            int x = button.rect.x() + (button.rect.width() - button.iconSize.width()) / 2;
            int y = button.rect.y() + (button.rect.height() - button.iconSize.height()) / 2;
            QRect targetRect(x, y, button.iconSize.width(), button.iconSize.height());
            QIcon::Mode mode = button.isHovered ? QIcon::Active : QIcon::Normal;
            button.icon.paint(&painter, targetRect, Qt::AlignCenter, mode);
        }
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
            if (button.useShapeDetection) {
                // 对于设置按钮，只检查图标区域
                int x = button.rect.x() + (button.rect.width() - button.iconSize.width()) / 2;
                int y = button.rect.y() + (button.rect.height() - button.iconSize.height()) / 2;
                QRect iconRect(x, y, button.iconSize.width(), button.iconSize.height());
                if (iconRect.contains(pos)) {
                    return &button;
                }
            } else {
                // 其他按钮保持原来的矩形检测
                if (button.rect.contains(pos)) {
                    return &button;
                }
            }
        }
        return nullptr;
    }

    void MainWindowMenuBar::showMenuAt(MenuItem *item) {
        if (!item || item == activeMenuItem_) {
            return;
        }

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
        activeMenu_->popup(pos);

        update();
    }

    void MainWindowMenuBar::hideActiveMenu() {
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
            int width = fm.horizontalAdvance(item.text) + 2 * BUTTON_PADDING;
            item.rect = QRect(x, 0, width, MENU_HEIGHT);
            x += width;
        }

        // 更新控制按钮位置
        x = width() - CONTROL_BUTTON_WIDTH * controlButtons_.size();
        for (auto &button: controlButtons_) {
            button.rect = QRect(x, 0, CONTROL_BUTTON_WIDTH, MENU_HEIGHT);
            x += CONTROL_BUTTON_WIDTH;
        }
    }

    void MainWindowMenuBar::updateControlButtonsPosition() {
        int x = width();
        // 从右到左依次放置按钮
        for (auto it = controlButtons_.rbegin(); it != controlButtons_.rend(); ++it) {
            it->rect = QRect(x - CONTROL_BUTTON_WIDTH, 0, CONTROL_BUTTON_WIDTH, MENU_HEIGHT);
            x -= CONTROL_BUTTON_WIDTH;
        }
    }
}
