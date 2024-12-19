//
// Created by wuxianggujun on 2024/12/18.
//

#include "CrashPaths.hpp"

#include <QDir>

namespace TinaToolBox {
    CrashPaths::CrashPaths(const QString &baseDir): baseDir_(baseDir) {
    }

    QString CrashPaths::getHandlerPath() const {
#ifdef Q_OS_WIN
        return baseDir_ + "/crashpad_handler.exe";
#else
return baseDir_ + "/crashpad_handler";
#endif
    }

    QString CrashPaths::getReportsPath() const {
        QString path = baseDir_ + "/crashes/reports";
        QDir().mkpath(path);  // 确保目录存在
        return path;
    }

    QString CrashPaths::getMetricsPath() const {
        QString path = baseDir_ + "/crashes/metrics";
        QDir().mkpath(path);  // 确保目录存在
        return path;
    }

    QString CrashPaths::getAttachmentPath() const {
        return baseDir_ + "/crashes/attachments";
    }

#if defined(Q_OS_UNIX)
    std::string CrashPaths::getPlatformString(const QString& string){
        return string.toStdString();
    }
#elif defined(Q_OS_WINDOWS)
    std::wstring CrashPaths::getPlatformString(const QString &string) {
        return string.toStdWString();
    }
#else
#error getPlatformString not implemented on this platform
#endif
}
