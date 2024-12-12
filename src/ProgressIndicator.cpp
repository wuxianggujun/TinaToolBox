#include "ProgressIndicator.hpp"
#include <QPainter>

namespace TinaToolBox {
    ProgressIndicator::ProgressIndicator(QWidget* parent)
        : QWidget(parent)
        , angle_(0)
        , timerId_(-1)
        , delay_(40)
        , displayedWhenStopped_(false)
        , color_(Qt::black)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setFocusPolicy(Qt::NoFocus);
    }

    bool ProgressIndicator::isAnimated() const {
        return (timerId_ != -1);
    }

    void ProgressIndicator::setDisplayedWhenStopped(bool state) {
        displayedWhenStopped_ = state;
        update();
    }

    bool ProgressIndicator::isDisplayedWhenStopped() const {
        return displayedWhenStopped_;
    }

    void ProgressIndicator::startAnimation() {
        angle_ = 0;
        if (timerId_ == -1) {
            timerId_ = startTimer(delay_);
            update();  // 立即触发一次重绘
        }
    }

    void ProgressIndicator::stopAnimation() {
        if (timerId_ != -1)
            killTimer(timerId_);
        timerId_ = -1;
        update();
    }

    void ProgressIndicator::setAnimationDelay(int delay) {
        if (delay >= 0) {
            delay_ = delay;
            if (timerId_ != -1) {
                killTimer(timerId_);
                timerId_ = startTimer(delay_);
            }
        }
    }

    void ProgressIndicator::setColor(const QColor& color) {
        color_ = color;
        update();
    }

    const QColor& ProgressIndicator::color() const {
        return color_;
    }

    int ProgressIndicator::animationDelay() const {
        return delay_;
    }

    QSize ProgressIndicator::minimumSizeHint() const {
        return {32,32};
    }

    void ProgressIndicator::timerEvent(QTimerEvent* /*event*/) {
        angle_ = (angle_ + 30) % 360;
        update();
    }

    void ProgressIndicator::paintEvent(QPaintEvent* /*event*/) {
        if (!isAnimated() && !displayedWhenStopped_)
            return;

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const int width = qMin(this->width(), this->height());
        const int outerRadius = (width - 1) * 0.5;
        const int innerRadius = (width - 1) * 0.5 * 0.38;

        const int capsuleHeight = outerRadius - innerRadius;
        const int capsuleWidth = (int)(capsuleHeight * 0.23);
        const int capsuleRadius = capsuleWidth / 2;

        // 计算中心点
        const QPoint center = rect().center();

        for (int i = 0; i < 12; i++) {
            QColor color = color_;
            color.setAlphaF(1.0f - (i / 12.0f));
            p.setPen(Qt::NoPen);
            p.setBrush(color);
            p.save();
            p.translate(center);
            p.rotate(angle_ - i * 30.0f);
            p.drawRoundedRect(-capsuleWidth * 0.5,
                             -(innerRadius + capsuleHeight),
                             capsuleWidth,
                             capsuleHeight,
                             capsuleRadius,
                             capsuleRadius);
            p.restore();
        }
    }
}