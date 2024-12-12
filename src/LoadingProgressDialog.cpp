#include "LoadingProgressDialog.hpp"
#include <QApplication>

namespace TinaToolBox {
    LoadingProgressDialog::LoadingProgressDialog(QWidget* parent)
        : QDialog(parent, Qt::FramelessWindowHint)
        , progressBar_(new QProgressBar(this))
        , statusLabel_(new QLabel(this))
    {
        setWindowTitle(tr("Loading..."));
        setFixedSize(300, 100);
        
        auto* layout = new QVBoxLayout(this);
        layout->addWidget(statusLabel_);
        layout->addWidget(progressBar_);
        
        progressBar_->setMinimum(0);
        progressBar_->setMaximum(100);
        progressBar_->setTextVisible(true);
        
        statusLabel_->setAlignment(Qt::AlignCenter);
        setModal(true);
    }

    void LoadingProgressDialog::startProgress(const QString& title) {
        setWindowTitle(title);
        progressBar_->setValue(0);
        show();
        QApplication::processEvents();
    }

    void LoadingProgressDialog::updateProgress(int value, const QString& status) {
        progressBar_->setValue(value);
        statusLabel_->setText(status);
        QApplication::processEvents();
    }

    void LoadingProgressDialog::finishProgress() {
        hide();
        QApplication::processEvents();
    }
}