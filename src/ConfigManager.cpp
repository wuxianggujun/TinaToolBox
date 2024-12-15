#include "ConfigManager.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <spdlog/spdlog.h>

namespace TinaToolBox {
    void ConfigManager::initialize() {
        configPath_ = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.toml";
        loadConfig(configPath_);
    }

    void ConfigManager::shutdown() {
        if (config_) {
            saveConfig();
            config_.reset();
        }
    }

    bool ConfigManager::loadConfig(const QString &configPath) {
        try {
            if (QFile::exists(configPath)) {
                config_ = std::make_shared<toml::table>(toml::parse_file(configPath.toStdString()));
            } else {
                // 创建新的配置
                config_ = std::make_shared<toml::table>();
                toml::table &root = *config_->as_table();

                // 应用程序基本配置
                root.insert_or_assign("app", toml::table{
                                          {"version", "0.1.0"},
                                          {"theme", "light"},
                                          {"language", "en"}
                                      });

                // 窗口配置
                root.insert_or_assign("window", toml::table{
                                          {"width", 1280},
                                          {"height", 800},
                                          {"maximized", false},
                                          {"x", 100},
                                          {"y", 100}
                                      });

                // 编辑器配置
                root.insert_or_assign("editor", toml::table{
                                          {"font_family", "Consolas"},
                                          {"font_size", 12},
                                          {"tab_size", 4},
                                          {"show_line_numbers", true},
                                          {"word_wrap", false},
                                          {"auto_indent", true}
                                      });


                // 创建配置目录并保存
                QFileInfo fileInfo(configPath);
                if (!QDir().mkpath(fileInfo.absolutePath())) {
                    spdlog::error("Failed to create config directory: {}",
                                  fileInfo.absolutePath().toStdString());
                    return false;
                }
                return saveConfig();
            }
            return true;
        } catch (const toml::parse_error &error) {
            spdlog::error("Failed to parse config file: {}", error.description());
            return false;
        }catch (const std::exception &e) {
            spdlog::error("Config error: {}", e.what());
            return false;
        }
    }

    bool ConfigManager::saveConfig() const {
        try {
            std::ofstream file(configPath_.toStdString());
            file << config_;
            return true;
        } catch (const std::exception &e) {
            spdlog::error("Failed to save config: {}", e.what());
            return false;
        }
    }

    QString ConfigManager::getString(const QString& section, const QString& key, 
                                   const QString& defaultValue) const {
        try {
            if (auto* node = config_->get_as<toml::table>(section.toStdString())) {
                if (auto* value = node->get_as<std::string>(key.toStdString())) {
                    return QString::fromStdString(value->get());
                }
            }
        } catch (const std::exception& e) {
            spdlog::warn("Failed to get string value for {}.{}: {}", 
                         section.toStdString(), key.toStdString(), e.what());
        }
        return defaultValue;
    }
int ConfigManager::getInt(const QString& section, const QString& key, 
                         int defaultValue) const {
    try {
        if (auto* node = config_->get_as<toml::table>(section.toStdString())) {
            if (auto* value = node->get_as<int64_t>(key.toStdString())) {
                return static_cast<int>(value->get());
            }
        }
    } catch (const std::exception& e) {
        spdlog::warn("Failed to get int value for {}.{}: {}", 
                     section.toStdString(), key.toStdString(), e.what());
    }
    return defaultValue;
}

bool ConfigManager::getBool(const QString& section, const QString& key, 
                          bool defaultValue) const {
    try {
        if (auto* node = config_->get_as<toml::table>(section.toStdString())) {
            if (auto* value = node->get_as<bool>(key.toStdString())) {
                return value->get();
            }
        }
    } catch (const std::exception& e) {
        spdlog::warn("Failed to get bool value for {}.{}: {}", 
                     section.toStdString(), key.toStdString(), e.what());
    }
    return defaultValue;
}

void ConfigManager::setString(const QString& section, const QString& key, 
                            const QString& value) {
    try {
        auto& sectionTable = config_->insert_or_assign(section.toStdString(), 
                                                      toml::table{}).first->second;
        sectionTable.as_table()->insert_or_assign(key.toStdString(), value.toStdString());
    } catch (const std::exception& e) {
        spdlog::error("Failed to set string value for {}.{}: {}", 
                      section.toStdString(), key.toStdString(), e.what());
    }
}

void ConfigManager::setInt(const QString& section, const QString& key, int value) {
    try {
        auto& sectionTable = config_->insert_or_assign(section.toStdString(), 
                                                      toml::table{}).first->second;
        sectionTable.as_table()->insert_or_assign(key.toStdString(), value);
    } catch (const std::exception& e) {
        spdlog::error("Failed to set int value for {}.{}: {}", 
                      section.toStdString(), key.toStdString(), e.what());
    }
}

void ConfigManager::setBool(const QString& section, const QString& key, bool value) {
        try {
            auto& sectionTable = config_->insert_or_assign(section.toStdString(), 
                                                          toml::table{}).first->second;
            sectionTable.as_table()->insert_or_assign(key.toStdString(), value);
        } catch (const std::exception& e) {
            spdlog::error("Failed to set bool value for {}.{}: {}", 
                          section.toStdString(), key.toStdString(), e.what());
        }
    }
    
}
