#pragma once

#include <QWidget>
#include <QMenu>
#include <QMap>

namespace TinaToolBox {
    class MainWindowMenuBar : public QWidget {
        Q_OBJECT

    public:
        explicit MainWindowMenuBar(QWidget *parent = nullptr);

        void updateMaximizeButton(bool isMaximized);

        bool isInDraggableArea(const QPoint& pos) const;
    signals:
        void menuActionTriggered(const QString &actionName);

        void minimizeClicked();

        void maximizeClicked();

        void closeClicked();

        void settingsClicked();
    

    protected:
        void paintEvent(QPaintEvent *event) override;

        void mousePressEvent(QMouseEvent *event) override;

        void mouseMoveEvent(QMouseEvent *event) override;

        void mouseReleaseEvent(QMouseEvent *event) override;

        void leaveEvent(QEvent *event) override;

        bool eventFilter(QObject *watched, QEvent *event) override;

        void resizeEvent(QResizeEvent *event) override;
    private:
        struct MenuItem {
            QString text;
            QRect rect;
            bool isHovered = false;
            bool isActive = false;
            QMenu *menu = nullptr;
        };

        struct ControlButton {
            QString name;
            QRect rect;
            bool isHovered = false;
            QIcon icon;
            QSize iconSize;
            bool useShapeDetection = false;
        };
        
        void initializeMenus();

        void initializeControlButtons();

        void drawMenuItem(QPainter &painter, const MenuItem &item);

        void drawControlButton(QPainter &painter, const ControlButton &button);

        MenuItem *getMenuItemAt(const QPoint &pos);

        ControlButton *getControlButtonAt(const QPoint &pos);

        void showMenuAt(MenuItem *item);

        void hideActiveMenu();

        void updateLayout();
        void updateControlButtonsPosition();

    private:
        QList<MenuItem> menuItems_;
        QList<ControlButton> controlButtons_;
        MenuItem *activeMenuItem_ = nullptr;
        ControlButton *hoveredButton_ = nullptr;
        QMenu *activeMenu_ = nullptr;
        bool isMaximized_ = false;

        const int MENU_HEIGHT = 35;
        const int BUTTON_PADDING = 10;
        const int CONTROL_BUTTON_WIDTH = 45;
        int MENU_ITEM_WIDTH = 50;

        // 标准尺寸参考
        const int ICON_SIZE = 16;  // 基础尺寸
        
        const QColor backgroundColor = QColor(0xF0F0F0);
        const QColor hoverColor = QColor(0xE8E8E8);
        const QColor activeColor = QColor(0xE0E0E0);
        const QColor textColor = QColor("#000000");
        const QColor closeHoverColor = QColor(0xE81123);
        const QColor lineColor = QColor(200, 200, 200);  // 浅灰色线条颜色
    };
}
