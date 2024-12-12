#pragma once

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>

namespace TinaToolBox {
    class LoadingProgressDialog : public QDialog {
        Q_OBJECT
    public:
        static LoadingProgressDialog* getInstance() {
            static LoadingProgressDialog instance;
            return &instance;
        }

        void startProgress(const QString& title);
        void updateProgress(int value, const QString& status);
        void finishProgress();

    protected:
        // ��ֹ�ⲿ����ʵ��
        LoadingProgressDialog(QWidget* parent = nullptr);
        ~LoadingProgressDialog() override = default;

    private:
        // ��ֹ�����͸�ֵ
        LoadingProgressDialog(const LoadingProgressDialog&) = delete;
        LoadingProgressDialog& operator=(const LoadingProgressDialog&) = delete;

        QProgressBar* progressBar_;
        QLabel* statusLabel_;
    };
}