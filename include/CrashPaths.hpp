//
// Created by wuxianggujun on 2024/12/18.
// https://github.com/BugSplat-Git/my-qt-crasher.git
//

#pragma once

#include <QString>

namespace TinaToolBox {
    class CrashPaths {
    public:
        explicit CrashPaths(const QString &baseDir);

        QString getHandlerPath() const;

        QString getReportsPath() const;

        QString getMetricsPath() const;

        QString getAttachmentPath() const;

#if defined(Q_OS_UNIX)
        static std::string getPlatformString(const QString &string);
#elif defined(Q_OS_WINDOWS)
        static std::wstring getPlatformString(const QString &string);
#else
#error getPlatformString not implemented on this platform
#endif

    private:
        QString baseDir_;
    };
} // TinaToolBox
