#pragma once

#include "Singleton.hpp"
#include <QString>
#include <QObject>
#include <memory>
#include <toml++/toml.hpp>

namespace TinaToolBox {

    class ConfigManager final : public QObject,public Singleton<ConfigManager> {
        Q_OBJECT
        friend class Singleton<ConfigManager>; // 允许基类访问私有构造函数
    public:
        bool initialized(const QString& configPath = "config.toml");
        bool saveConfig();

        QString getString(const QString& section,const QString& key, const QString& defaultValue = "") const;
        int getInt(const QString& section,const QString& key, int defaultValue = 0) const;
        bool getBool(const QString& section,const QString& key, bool defaultValue = false) const;

        void setString(const QString& section,const QString& key, const QString& value);
        void setInt(const QString& section,const QString& key, int value);
        void setBool(const QString& section,const QString& key, bool value);

    private:
        ConfigManager() : QObject(nullptr) {}

        QString configPath_;
        std::shared_ptr<toml::table> config_;
        
    };

    
}