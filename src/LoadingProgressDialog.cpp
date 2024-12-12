#include "LoadingProgressDialog.hpp"
#include <QApplication>

namespace TinaToolBox {
    LoadingProgressDialog::LoadingProgressDialog(QWidget *parent):QDialog(parent,Qt::FramelessWindowHint),progressBar_(new QProgressBar(this)),statusLabel_(new QLabel(this)) {
        setWindowTitle("Loading...");
        setFixedSize(300,100);

        auto* layout = new QVBoxLayout(this);
        layout->addWidget(statusLabel_);
        layout->addWidget(progressBar_);

        progressBar_->setMinimum(0);
        progressBar_->setMaximum(100);
        progressBar_->setTextVisible(true);

        statusLabel_->setAlignment(Qt::AlignCenter);
        setModal(true);
    }

    void LoadingProgressDialog::setProgress(int value) {
        progressBar_->setValue(value);
        QApplication::processEvents();
    }

    void LoadingProgressDialog::setMaximum(int max) {
        progressBar_->setMaximum(max);
    }

    void LoadingProgressDialog::setStatusText(const QString &text) {
        statusLabel_->setText(text);
        QApplication::processEvents();
    }
}
