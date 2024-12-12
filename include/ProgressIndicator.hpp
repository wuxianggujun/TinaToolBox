#pragma once

#include <QWidget>
#include <QColor>

namespace TinaToolBox {
    class ProgressIndicator : public QWidget {
        Q_OBJECT
        Q_PROPERTY(int delay READ animationDelay WRITE setAnimationDelay)
        Q_PROPERTY(bool displayedWhenStopped READ isDisplayedWhenStopped WRITE setDisplayedWhenStopped)
        Q_PROPERTY(QColor color READ color WRITE setColor)

    public:
        explicit ProgressIndicator(QWidget* parent = nullptr);

        bool isAnimated() const;
        bool isDisplayedWhenStopped() const;
        const QColor& color() const;
        int animationDelay() const;
        QSize minimumSizeHint() const override;

        public slots:
            void startAnimation();
        void stopAnimation();
        void setAnimationDelay(int delay);
        void setDisplayedWhenStopped(bool state);
        void setColor(const QColor& color);

    protected:
        void paintEvent(QPaintEvent* event) override;
        void timerEvent(QTimerEvent* event) override;

    private:
        int angle_;
        int timerId_;
        int delay_;
        bool displayedWhenStopped_;
        QColor color_;
    };
}