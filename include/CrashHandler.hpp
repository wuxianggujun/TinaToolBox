//
// Created by wuxianggujun on 2024/12/18.
//

#pragma  once
#include <QString>
#include <memory>
#include <client/crashpad_client.h>
#include <client/crash_report_database.h>
#include <client/settings.h>

namespace TinaToolBox {
    class CrashHandler {
    public:
        static bool initializeCrashpad(QString dbName, QString appName, QString appVersion);

    private:
       static  QString getExecutableDir();
    };
} // TinaToolBox
