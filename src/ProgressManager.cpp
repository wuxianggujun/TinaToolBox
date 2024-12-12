#include "ProgressManager.hpp"
#include <QApplication>

namespace TinaToolBox {
    ProgressManager &ProgressManager::getInstance() {
        static ProgressManager instance;
        return instance;
    }

    void ProgressManager::startProgress(const QString &title, int maximum) {
        if (!progressDialog_) {
            progressDialog_ = std::make_unique<LoadingProgressDialog>(QApplication::activeWindow());
        }

        progressDialog_->setWindowTitle(title);
        progressDialog_->setMaximum(maximum);
        progressDialog_->setProgress(0);
        progressDialog_->show();
    }

    void ProgressManager::updateProgress(int value, const QString &status) {
        if (progressDialog_) {
            progressDialog_->setProgress(value);
            if (!status.isEmpty()) {
                progressDialog_->setStatusText(status);
            }
        }
    }

    void ProgressManager::finishProgress() {
        if (progressDialog_) {
            progressDialog_->hide();
        }
    }
}
