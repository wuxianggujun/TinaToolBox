#pragma once

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>

namespace TinaToolBox {
    class LoadingProgressDialog : public QDialog {
        Q_OBJECT
    public:
        static LoadingProgressDialog& getInstance() {
            static LoadingProgressDialog instance;
            return instance;
        }

        void startProgress(const QString& title);
        void updateProgress(int value, const QString& status);
        void finishProgress();

    private:
        LoadingProgressDialog(QWidget* parent = nullptr);
        ~LoadingProgressDialog() override = default;

        QProgressBar* progressBar_;
        QLabel* statusLabel_;
    };
}