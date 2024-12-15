#include "MainWindow.hpp"
#include "LogSystem.hpp"
#include "Singleton.hpp"
#include <QApplication>

using namespace TinaToolBox;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SingletonGuard<LogSystem> logSystemGuard_;
    MainWindow w;
    w.show();
    return a.exec();
}
