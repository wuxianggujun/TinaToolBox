#pragma once

#include "Singleton.hpp"
#include <QString>
#include <QObject>
#include <memory>
#include <spdlog/spdlog.h>
#include <toml++/toml.hpp>

namespace TinaToolBox {
    class ConfigManager final : public QObject, public Singleton<ConfigManager> {
        Q_OBJECT
        friend class Singleton<ConfigManager>; // 允许基类访问私有构造函数
    public:
        void initialize() override;

        void shutdown() override;

        bool loadConfig(const QString &configPath);

        bool saveConfig() const;

        QString getString(const QString &section, const QString &key, const QString &defaultValue = "") const;

        int getInt(const QString &section, const QString &key, int defaultValue = 0) const;

        bool getBool(const QString &section, const QString &key, bool defaultValue = false) const;

        void setString(const QString &section, const QString &key, const QString &value);

        void setInt(const QString &section, const QString &key, int value);

        void setBool(const QString &section, const QString &key, bool value);

    private:
        template<typename T>
        T getValue(const QString &section, const QString &key, const T &defaultValue) const {
            try {
                if (auto *node = config_->get_as<toml::table>(section.toStdString())) {
                    if (auto *value = node->get_as<T>(key.toStdString())) {
                        return value->get();
                    }
                }
            } catch (const std::exception &e) {
                spdlog::warn("Failed to get value for {}.{}: {}",
                             section.toStdString(), key.toStdString(), e.what());
            }
            return defaultValue;
        }

        // 用于设置值的模板方法
        template<typename T>
        void setValue(const QString &section, const QString &key, const T &value) {
            try {
                auto &sectionTable = config_->insert_or_assign(section.toStdString(),
                                                               toml::table{}).first->second;
                sectionTable.as_table()->insert_or_assign(key.toStdString(), value);
            } catch (const std::exception &e) {
                spdlog::error("Failed to set value for {}.{}: {}",
                              section.toStdString(), key.toStdString(), e.what());
            }
        }

        ConfigManager() : QObject(nullptr) {
        }

        QString configPath_;
        std::shared_ptr<toml::table> config_;
    };
}
