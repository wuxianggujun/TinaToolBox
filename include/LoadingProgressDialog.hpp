#pragma once

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>

namespace TinaToolBox {
    class LoadingProgressDialog : public QDialog {
        Q_OBJECT

    public:
        explicit LoadingProgressDialog(QWidget *parent = nullptr);

        void setProgress(int value);

        void setMaximum(int max);

        void setStatusText(const QString &text);

    private:
        QProgressBar *progressBar_;
        QLabel *statusLabel_;
    };
}
