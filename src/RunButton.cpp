//
// Created by wuxianggujun on 2024/11/24.
//

#include "RunButton.hpp"


RunButton::RunButton(QWidget* parent) : QPushButton(parent), isRunning_(true) {
    // 设置固定大小
    // 设置颜色
    runColor_ = QColor(0x4CAF50);    // 绿色
    pauseColor_ = QColor(0xFF0000);   // 红色

    // 设置按钮属性
    setFlat(true);  // 设置为平面按钮
    setCursor(Qt::PointingHandCursor);  // 设置鼠标悬停时的光标

    // 设置工具提示
    setToolTip(tr("运行"));
    QFont tooltipFont;
    tooltipFont.setPointSize(10);
    QToolTip::setFont(tooltipFont);

    // 连接点击信号
    connect(this, &RunButton::clicked, this, &RunButton::toggleState);
}

void RunButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 获取按钮中心点
    int centerX = width() / 2;
    int centerY = height() / 2;

    // 设置当前颜色
    QColor color = isRunning_ ? runColor_ : pauseColor_;
    painter.setBrush(color);
    painter.setPen(QPen(color));

    if (isRunning_) {
        // 绘制三角形（运行状态）
        QPolygon points;
        points << QPoint(centerX - 8, centerY - 8)   // 左上角
              << QPoint(centerX - 8, centerY + 8)    // 左下角
              << QPoint(centerX + 8, centerY);       // 右中点
        painter.drawPolygon(points);
    } else {
        // 绘制暂停符号（两个竖条）
        QRect rect1(centerX - 8, centerY - 8, 5, 16);
        QRect rect2(centerX + 3, centerY - 8, 5, 16);
        painter.drawRect(rect1);
        painter.drawRect(rect2);
    }
}

void RunButton::toggleState() {
    isRunning_ = !isRunning_;
    // 更新工具提示文本
    setToolTip(isRunning_ ? tr("运行") : tr("暂停"));
    emit stateChanged(isRunning_);
    update();
}

void RunButton::setState(bool isRunning) {
    if (isRunning_ != isRunning) {
        isRunning_ = isRunning;
        setToolTip(isRunning_ ? tr("运行") : tr("暂停"));
        emit stateChanged(isRunning_);
        update();
    }
}

void RunButton::setColors(const QString& runColor, const QString& pauseColor) {
    runColor_ = QColor(runColor);
    pauseColor_ = QColor(pauseColor);
    update();
}

void RunButton::enterEvent(QEnterEvent* event) {
    QToolTip::showText(
        mapToGlobal(rect().bottomLeft()),
        isRunning_ ? tr("运行") : tr("暂停")
    );
    QPushButton::enterEvent(event);
}

void RunButton::leaveEvent(QEvent* event) {
    QToolTip::hideText();
    QPushButton::leaveEvent(event);
}