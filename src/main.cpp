#include <iostream>

#include "MainWindow.hpp"
#include "LogSystem.hpp"
#include "Singleton.hpp"
#include "CrashHandler.hpp"
#include <QApplication>
#include <cxxbridge-cpp/foo/mod.h>
#include <cxxbridge-cpp/lib.h>
#include <vector>

using namespace TinaToolBox;

int main(int argc, char *argv[]) {

    std::string file_path = "C:/Users/wuxianggujun/Downloads/工单查询 (11).xlsx"; // 替换为你的 XLSX 文件路径

    bool success = lib::open_xlsx_file(file_path);

    if (success) {
        qDebug() << "Successfully opened the XLSX file from Rust!" ;
    } else {
        qDebug() << "Failed to open the XLSX file from Rust."; 
    }
    
    QString appName = "TinaToolBox";
    QString appVersion = "1.1";

    CrashHandler::initializeCrashpad(appName, appVersion);
    
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
