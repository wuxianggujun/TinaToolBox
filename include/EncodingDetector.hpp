#pragma once
#include <QString>
#include <QByteArray>

namespace TinaToolBox {
    class EncodingDetector {
    public:
        // 禁用构造函数，使其成为纯静态类
        EncodingDetector() = delete;
        
        // 静态方法检测编码
        static QString detect(const QByteArray& data);
        
    private:
        // 辅助检测方法
        static bool isValidUtf8(const QByteArray& data);
        static bool hasChineseChars(const QByteArray& data);
    };
}