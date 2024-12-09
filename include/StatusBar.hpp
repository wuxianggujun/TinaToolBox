#pragma once

#include <QWidget>
#include <QLabel>
#include <QMenu>
#include <QGuiApplication>
#include <QScreen>

namespace TinaToolBox {
    class StatusBar : public QWidget {
        Q_OBJECT

    public:
        explicit StatusBar(QWidget *parent = nullptr);

        ~StatusBar() override;

        void setFilePath(const QString &filePath);

        void setEncoding(const QString &encoding);

        void setEncodingVisible(bool visible);
        
    signals:
        void encodingChanged(const QString &encoding);

    protected:
        void mousePressEvent(QMouseEvent *event) override;

    private:
        void setupUI();

        void createEncodingMenu();

        QLabel *filePathLabel_;
        QLabel *encodingLabel_;
        QMenu *encodingMenu_;
    };
}
