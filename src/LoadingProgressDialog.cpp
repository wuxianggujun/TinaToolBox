#include "LoadingProgressDialog.hpp"
#include <QApplication>
#include <QScreen>

namespace TinaToolBox {

    LoadingProgressDialog::LoadingProgressDialog(QWidget* parent)
        : QDialog(parent, Qt::FramelessWindowHint)
        , statusLabel_(new QLabel(this)),spinner_(new ProgressIndicator(this)) // ???????
    {
        setWindowTitle(tr("Loading..."));
        setFixedSize(300, 100);
        
        auto* layout = new QVBoxLayout(this);
        layout->setSpacing(10);
    
        // ??spinner???????
        spinner_->setFixedSize(32, 32);
        spinner_->setColor(QColor(0, 120, 212));  // ????
        layout->addWidget(spinner_, 0, Qt::AlignCenter);

        
        // ?????????????
        statusLabel_->setAlignment(Qt::AlignCenter);
        layout->addWidget(statusLabel_, 0, Qt::AlignCenter);
    
        setModal(true);
        
        if (parent) {
            setParent(parent);
        }
        else {
            setWindowFlags(windowFlags() | Qt::Dialog);
        }
    
    }

    void LoadingProgressDialog::startProgress(const QString& title,bool useSpinner) {
        setWindowTitle(title);
        statusLabel_->setText(title);
        spinner_->startAnimation();
        
        
        if (parentWidget()) {
            move(parentWidget()->window()->frameGeometry().center() -
                rect().center());
        }
        else {
            QRect screenGeometry = QApplication::primaryScreen()->geometry();
            move(screenGeometry.center() - rect().center());
        }

        show();
        QApplication::processEvents();
    }

    void LoadingProgressDialog::updateProgress(int value, const QString& status) {
        statusLabel_->setText(status);
        QApplication::processEvents();
    }

    void LoadingProgressDialog::finishProgress() {
        hide();
        spinner_->stopAnimation();
        statusLabel_->clear();
        QApplication::processEvents();
    }
}
