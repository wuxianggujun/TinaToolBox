#include "CrashHandler.hpp"

#if defined(Q_OS_WIN)
    #include <windows.h>
#endif

#if defined(Q_OS_MAC)
    #include <mach-o/dyld.h>
#endif

#if defined(Q_OS_LINUX)
    #include <unistd.h>
    #define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <vector>
#include "CrashPaths.hpp"
#include "client/crash_report_database.h"
#include "client/crashpad_client.h"
#include "client/settings.h"

namespace TinaToolBox {
    using namespace base;
    using namespace crashpad;
    
    bool CrashHandler::initializeCrashpad(QString dbName, QString appName, QString appVersion)
    {
        // Get directory where the exe lives so we can pass a full path to handler, reportsDir and metricsDir
        // QString exeDir = getExecutableDir();
        QString exeDir = QCoreApplication::applicationDirPath();

        // Helper class for cross-platform file systems
        CrashPaths crashpadPaths(exeDir);

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
        if (database == nullptr) return false;

        // Enable automated crash uploads
        Settings *settings = database->GetSettings();
        if (settings == nullptr) return false;
        // 禁用自动上传
        settings->SetUploadsEnabled(true);

        // Attachments to be uploaded alongside the crash - default bundle size limit is 20MB
        std::vector<FilePath> attachments;
        FilePath attachment(CrashPaths::getPlatformString(crashpadPaths.getAttachmentPath()));
        attachments.push_back(attachment);

        // Start crash handler
        const auto client = new CrashpadClient();
        bool status = client->StartHandler(handler, reportsDir, metricsDir, "", annotations.toStdMap(), arguments, true, true, attachments);
        return status;
    }

    QString CrashHandler::getExecutableDir() {
#if defined(Q_OS_MAC)
        unsigned int bufferSize = 512;
        std::vector<char> buffer(bufferSize + 1);

        if(_NSGetExecutablePath(&buffer[0], &bufferSize))
        {
            buffer.resize(bufferSize);
            _NSGetExecutablePath(&buffer[0], &bufferSize);
        }

        char* lastForwardSlash = strrchr(&buffer[0], '/');
        if (lastForwardSlash == NULL) return NULL;
        *lastForwardSlash = 0;

        return &buffer[0];
#elif defined(Q_OS_WINDOWS)
        HMODULE hModule = GetModuleHandleW(nullptr);
        WCHAR path[MAX_PATH];
        DWORD retVal = GetModuleFileNameW(hModule, path, MAX_PATH);
        if (retVal == 0) return nullptr;

        wchar_t *lastBackslash = wcsrchr(path, '\\');
        if (lastBackslash == nullptr) return nullptr;
        *lastBackslash = 0;

        return QString::fromWCharArray(path);
#elif defined(Q_OS_LINUX)
        char pBuf[FILENAME_MAX];
        int len = sizeof(pBuf);
        int bytes = MIN(readlink("/proc/self/exe", pBuf, len), len - 1);
        if (bytes >= 0) {
            pBuf[bytes] = '\0';
        }

        char* lastForwardSlash = strrchr(&pBuf[0], '/');
        if (lastForwardSlash == NULL) return NULL;
        *lastForwardSlash = '\0';

        return QString::fromStdString(pBuf);
#else
#error getExecutableDir not implemented on this platform
#endif
    }
}