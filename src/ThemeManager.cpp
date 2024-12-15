//#include "ThemeManager.hpp"
//#include "ConfigManager.hpp"
//#include <QFile>
//#include <QJsonDocument>
//#include <QJsonObject>
//#include <QJsonArray>
//#include <spdlog/spdlog.h>
//
//namespace TinaToolBox {
//
//void ThemeManager::loadTheme(const QString& themeName) {
//    // 清除旧的主题数据
//    colors_.clear();
//    stylesheets_.clear();
//
//    // 构建主题文件路径（假设主题文件存储在 themes 目录下）
//    QString themePath = QString(":/themes/%1.json").arg(themeName);
//    
//    QFile themeFile(themePath);
//    if (!themeFile.open(QIODevice::ReadOnly)) {
//        spdlog::error("Failed to open theme file: {}", themePath.toStdString());
//        return;
//    }
//
//    // 读取并解析 JSON
//    QJsonDocument doc = QJsonDocument::fromJson(themeFile.readAll());
//    if (doc.isNull()) {
//        spdlog::error("Invalid theme file format");
//        return;
//    }
//
//    QJsonObject root = doc.object();
//
//    // 加载颜色
//    if (root.contains("colors")) {
//        QJsonObject colors = root["colors"].toObject();
//        for (auto it = colors.begin(); it != colors.end(); ++it) {
//            colors_[it.key()] = QColor(it.value().toString());
//        }
//    }
//
//    // 加载样式表
//    if (root.contains("stylesheets")) {
//        QJsonObject stylesheets = root["stylesheets"].toObject();
//        for (auto it = stylesheets.begin(); it != stylesheets.end(); ++it) {
//            stylesheets_[it.key()] = it.value().toString();
//        }
//    }
//
//    currentTheme_ = themeName;
//    
//    // 保存当前主题到配置
//    ConfigManager::getInstance().setString("app", "theme", themeName);
//    ConfigManager::getInstance().saveConfig();
//
//    // 发送主题改变信号
//    emit themeChanged();
//}
//
//QColor ThemeManager::getColor(const QString& colorName) const {
//    return colors_.value(colorName, QColor()); // 如果找不到返回无效颜色
//}
//
//QString ThemeManager::getStyleSheet(const QString& styleSheetName) const {
//    return stylesheets_.value(styleSheetName, QString()); // 如果找不到返回空字符串
//}
//
//}