#include "StatusBar.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>

namespace TinaToolBox {
    StatusBar::StatusBar(QWidget *parent): QWidget(parent) {
        setFixedHeight(25);
        setupUI();
        createEncodingMenu();
    }

    StatusBar::~StatusBar() {
        delete encodingMenu_;
    }

    void StatusBar::setupUI() {
        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(5, 0, 5, 0);
        layout->setSpacing(5);

        // 文件路径标签
        filePathLabel_ = new QLabel(this);
        filePathLabel_->setStyleSheet(
            "color: #666666;"
            "font-size: 12px;"
        );
        layout->addWidget(filePathLabel_);

        layout->addStretch();

        // 编码标签
        encodingLabel_ = new QLabel(this);
        encodingLabel_->setStyleSheet(
            "color: #666666;"
            "font-size: 12px;"
            "padding: 2px 8px;"
            "border-radius: 2px;"
            "background: #f0f0f0;"
            "border: 1px solid #e0e0e0;"
            "cursor: pointer;"
        );
        encodingLabel_->setCursor(Qt::PointingHandCursor);
        encodingLabel_->hide(); // 初始状态隐藏编码标签
        layout->addWidget(encodingLabel_);

        // 设置整个状态栏的样式
        setStyleSheet(
            "StatusBar {"
            "    background-color: #f5f5f5;"
            "    border-top: 1px solid #e0e0e0;"
            "}"
        );
    }

    void StatusBar::createEncodingMenu() {
        encodingMenu_ = new QMenu(this);

        // 主要编码列表
        QStringList mainEncodings = {
            "ANSI",
            "UTF-8",
            "GB2312"
        };

        // 更多编码列表
        QStringList moreEncodings = {
            "UTF-16",
            "GB18030",
            "GBK",
            "Big5",
            "Latin1"
        };

        // 添加主要编码
        for (const auto &encoding: mainEncodings) {
            QAction *action = encodingMenu_->addAction(encoding);
            connect(action, &QAction::triggered, this, [this, encoding]() {
                setEncoding(encoding);
                emit encodingChanged(encoding);
            });
        }

        // 添加分隔线
        encodingMenu_->addSeparator();

        // 创建"更多编码"子菜单
        auto *moreMenu = new QMenu("更多编码", encodingMenu_);
        for (const auto &encoding: moreEncodings) {
            QAction *action = moreMenu->addAction(encoding);
            connect(action, &QAction::triggered, this, [this, encoding]() {
                setEncoding(encoding);
                emit encodingChanged(encoding);
            });
        }

        encodingMenu_->addMenu(moreMenu);
    }


    void StatusBar::mousePressEvent(QMouseEvent *event) {
        if (encodingLabel_->geometry().contains(event->pos())) {
            // 计算菜单显示位置：在编码标签正上方
            QPoint pos = encodingLabel_->mapToGlobal(QPoint(0, 0));
            pos.setY(pos.y() - encodingMenu_->sizeHint().height());

            // 确保菜单不会超出屏幕
            QScreen *screen = QGuiApplication::screenAt(pos);
            if (screen) {
                QRect screenGeometry = screen->geometry();
                if (pos.y() < screenGeometry.top()) {
                    // 如果菜单会超出屏幕顶部，就显示在标签下方
                    pos.setY(encodingLabel_->mapToGlobal(QPoint(0, encodingLabel_->height())).y());
                }
            }
            encodingMenu_->popup(pos);
        }
    }

    void StatusBar::setFilePath(const QString &path) const {
        if (!path.isEmpty()) {
            filePathLabel_->setText(path);
            filePathLabel_->show();
        } else {
            filePathLabel_->setText("");
            filePathLabel_->hide();
            // 当没有文件时，同时隐藏编码标签
            encodingLabel_->hide();
        }
    }

    void StatusBar::setEncoding(const QString &encoding) const {
        if (!encoding.isEmpty()) {
            encodingLabel_->setText(encoding);
            encodingLabel_->show();
        } else {
            encodingLabel_->hide();
        }
    }

    void StatusBar::setEncodingVisible(bool visible) const {
        encodingLabel_->setVisible(visible);
    }
    
}
