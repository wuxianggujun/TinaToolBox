//
// Created by wuxianggujun on 2024/11/24.
//

#ifndef TINA_TOOL_BOX_RUN_BUTTON_HPP
#define TINA_TOOL_BOX_RUN_BUTTON_HPP

#include <QPushButton>
#include <QColor>
#include <QPainter>
#include <QToolTip>
#include <QFont>
#include <QRect>

class RunButton : public QPushButton {
    Q_OBJECT
public:
    explicit RunButton(QWidget* parent = nullptr);

    void setState(bool isRunning);
    void setColors(const QString& runColor, const QString& pauseColor);

    signals:
        void stateChanged(bool isRunning);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

    private slots:
        void toggleState();

private:
    bool isRunning_;
    QColor runColor_;
    QColor pauseColor_;
};

#endif //TINA_TOOL_BOX_RUN_BUTTON_HPP
