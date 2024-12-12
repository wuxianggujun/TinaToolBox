#pragma once

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include "ProgressIndicator.hpp"

namespace TinaToolBox {
    class LoadingProgressDialog : public QDialog {
        Q_OBJECT
    public:
        static LoadingProgressDialog* getInstance() {
            static LoadingProgressDialog instance;
            return &instance;
        }

        void startProgress(const QString& title,bool useSpinner = false);
        void updateProgress(int value, const QString& status);
        void finishProgress();

    protected:
        LoadingProgressDialog(QWidget* parent = nullptr);
        ~LoadingProgressDialog() override = default;

    private:
        LoadingProgressDialog(const LoadingProgressDialog&) = delete;
        LoadingProgressDialog& operator=(const LoadingProgressDialog&) = delete;
        
        QLabel* statusLabel_;
        ProgressIndicator* spinner_;  // ?????????
    };
}