//
// Created by wuxianggujun on 2024/12/18.
//

#pragma  once
#include <QString>
#include "NonCopyable.hpp"

namespace TinaToolBox {
    class CrashHandler : public NonCopyable{
    public:
        static bool initializeCrashpad(const QString &appName, const QString &appVersion);

    private:
       static  QString getExecutableDir();
    };
} // TinaToolBox
