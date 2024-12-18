#include "MainWindow.hpp"
#include "LogSystem.hpp"
#include "Singleton.hpp"
#include "CrashHandler.hpp"
#include <QApplication>

using namespace TinaToolBox;

int main(int argc, char *argv[]) {

    QString dbName = "TinaToolBox";
    QString appName = "TinaToolBox";
    QString appVersion = "1.1";

    CrashHandler::initializeCrashpad(dbName, appName, appVersion);
    
    QApplication a(argc, argv);
    
    qDebug() << "Starting application...";

    auto &logSystem = LogSystem::getInstance();
    logSystem.initialize();

    // 确保 ConfigManager 在 LogSystem 之后初始化
    auto &configManager = ConfigManager::getInstance();
    configManager.initialize();

    MainWindow w;
    w.show();
    const int result = QApplication::exec();

    configManager.shutdown();
    logSystem.shutdown();
    return result;
}
