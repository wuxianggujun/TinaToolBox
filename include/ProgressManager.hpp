#pragma once

#include <QObject>
#include <memory>
#include <LoadingProgressDialog.hpp>

namespace TinaToolBox {
    class ProgressManager : public QObject {
        Q_OBJECT

    public:
        static ProgressManager &getInstance();

        void startProgress(const QString &title, int maximum = 100);

        void updateProgress(int value, const QString &status = QString());

        void finishProgress();

    private:
        ProgressManager() = default;

        ~ProgressManager() override = default;

        std::unique_ptr<LoadingProgressDialog> progressDialog_;
    };
}
