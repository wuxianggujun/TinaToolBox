#include "CrashHandler.hpp"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <vector>
#include "CrashPaths.hpp"
#include <memory>
#include <client/crashpad_client.h>
#include <client/crash_report_database.h>
#include <client/settings.h>

namespace TinaToolBox {
    using namespace base;
    using namespace crashpad;
    
    bool CrashHandler::initializeCrashpad(const QString &appName, const QString &appVersion)
    {
        // 使用当前工作目录
        QString currentDir = getExecutableDir();
        // Helper class for cross-platform file systems
        CrashPaths crashpadPaths(currentDir);

        // Ensure that crashpad_handler is shipped with your application
        FilePath handler(CrashPaths::getPlatformString(crashpadPaths.getHandlerPath()));

        // Directory where reports will be saved. Important! Must be writable or crashpad_handler will crash.
        FilePath reportsDir(CrashPaths::getPlatformString(crashpadPaths.getReportsPath()));

        // Directory where metrics will be saved. Important! Must be writable or crashpad_handler will crash.
        FilePath metricsDir(CrashPaths::getPlatformString(crashpadPaths.getMetricsPath()));
        
        // Metadata that will be posted to BugSplat
        QMap<std::string, std::string> annotations;
        annotations["format"] = "minidump";                 // Required: Crashpad setting to save crash as a minidump
        annotations["product"] = appName.toStdString();     // Required: BugSplat appName
        annotations["version"] = appVersion.toStdString();  // Required: BugSplat appVersion
        annotations["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate).toStdString();

        // Disable crashpad rate limiting so that all crashes have dmp files
        std::vector<std::string> arguments;
        arguments.push_back("--no-rate-limit");

        // Initialize crashpad database
        std::unique_ptr<CrashReportDatabase> database = CrashReportDatabase::Initialize(reportsDir);
        if (!database) {
            qDebug() << "Failed to initialize crash database";
            return false;
        }

        // 配置设置
        Settings* settings = database->GetSettings();
        if (!settings) {
            qDebug() << "Failed to get database settings";
            return false;
        }
        // 禁用自动上传
        settings->SetUploadsEnabled(false);
        
        // 启动处理程序
        const auto client = new CrashpadClient();
   
        // 这里传空字符串作为服务器 URL，表示不上传
        bool status = client->StartHandler(
            handler,
            reportsDir,
            metricsDir,
            "",  // 空 URL，禁用上传
            annotations.toStdMap(),
            arguments,
            true,  // 重启处理程序
            true,  // 异步启动
            {}     // 不需要附件
        );
        if (!status) {
            qDebug() << "Failed to start crashpad handler";
        }
        return status;
        
    }

    QString CrashHandler::getExecutableDir() {
        return QDir::currentPath();
    }
}
