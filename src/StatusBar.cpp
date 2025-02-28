#include "StatusBar.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <spdlog/spdlog.h>

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
        auto* layout = new QHBoxLayout(this);
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

        // 编码标签 - 移除 cursor 属性从样式表中
        encodingLabel_ = new QLabel(this);
        encodingLabel_->setStyleSheet(
            "color: #666666;"
            "font-size: 12px;"
            "padding: 2px 8px;"
            "border-radius: 2px;"
            "background: #f0f0f0;"
            "border: 1px solid #e0e0e0;"
        );
        // 直接设置鼠标样式
        encodingLabel_->setCursor(Qt::PointingHandCursor);
        encodingLabel_->hide();
        layout->addWidget(encodingLabel_);

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
            "UTF-8",
            "GBK",
            "GB2312",
            "GB18030",
            "Big5"
        };

        // 更多编码列表
        QStringList moreEncodings = {
            "UTF-16LE",
            "UTF-16BE",
            "Latin1",
            "ASCII"
        };

        // 添加主要编码
        for (const auto& encoding : mainEncodings) {
            encodingMenu_->addAction(encoding);
        }

        // 添加分隔线
        encodingMenu_->addSeparator();

        // 创建"更多编码"子菜单
        auto* moreMenu = new QMenu("更多编码", encodingMenu_);
        for (const auto& encoding : moreEncodings) {
            moreMenu->addAction(encoding);
        }

        encodingMenu_->addMenu(moreMenu);
    }


    void StatusBar::mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton && encodingLabel_->isVisible() && 
          encodingLabel_->geometry().contains(event->pos())) {
            spdlog::debug("Encoding label clicked");
        
            // 计算菜单显示位置
            QPoint pos = encodingLabel_->mapToGlobal(QPoint(0, 0));
            pos.setY(pos.y() - encodingMenu_->sizeHint().height());

            // 确保菜单不会超出屏幕
            QScreen *screen = QGuiApplication::screenAt(pos);
            if (screen) {
                QRect screenGeometry = screen->geometry();
                if (pos.y() < screenGeometry.top()) {
                    pos.setY(encodingLabel_->mapToGlobal(QPoint(0, encodingLabel_->height())).y());
                }
            }
    
            // 显示菜单并获取选择的动作
            QAction* action = encodingMenu_->exec(pos);
            if (action) {
                QString encoding = action->text();
                spdlog::debug("Menu item selected: {}", encoding.toStdString());
                setEncoding(encoding);
                emit encodingChanged(encoding);
            }
        
            event->accept();
            return;
          }
        QWidget::mousePressEvent(event);
    }

    void StatusBar::setFilePath(const QString &path) {
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

    void StatusBar::setEncoding(const QString &encoding) {
        if (!encoding.isEmpty()) {
            encodingLabel_->setText(encoding);
            encodingLabel_->show();
        } else {
            encodingLabel_->hide();
        }
    }

    void StatusBar::setEncodingVisible(bool visible) {
        encodingLabel_->setVisible(visible);
    }
    
}
