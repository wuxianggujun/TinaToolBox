#include "ConfigManager.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <spdlog/spdlog.h>

#include "LogSystem.hpp"

namespace TinaToolBox {
    void ConfigManager::initialize() {
        // 确保 LogSystem 已经初始化
        if (!LogSystem::getInstance().isInitialized()) {
            LogSystem::getInstance().initialize();
        }
    
        configPath_ = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.toml";
        loadConfig(configPath_);
    }

    void ConfigManager::shutdown() {
        if (config_) {
            saveConfig();
            config_.reset();
            config_ = nullptr;
        } 
    }
    
    bool ConfigManager::loadConfig(const QString &configPath) {
        try {
            // 检查配置文件是否存在
            if (QFile::exists(configPath)) {
                // 如果存在，直接加载
                config_ = std::make_shared<toml::table>(toml::parse_file(configPath.toStdString()));
                return true;
            } else {
                // 如果不存在，创建默认配置
                config_ = std::make_shared<toml::table>();
                toml::table &root = *config_;

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

                // 创建配置目录
                QFileInfo fileInfo(configPath);
                if (!QDir().mkpath(fileInfo.absolutePath())) {
                    spdlog::error("Failed to create config directory: {}", 
                        fileInfo.absolutePath().toStdString());
                    return false;
                }

                // 保存默认配置
                return saveConfig();
            }
        } catch (const toml::parse_error &error) {
            spdlog::error("Failed to parse config file: {}", error.description());
            return false;
        } catch (const std::exception &e) {
            spdlog::error("Config error: {}", e.what());
            return false;
        }
    }


    bool ConfigManager::saveConfig() const {
        try {
            // 打开文件并检查
            std::ofstream file(configPath_.toStdString());
            if (!file.is_open()) {
                spdlog::error("Failed to open config file for writing: {}", 
                    configPath_.toStdString());
                return false;
            }

            // 正确写入 TOML 内容
            file << *config_;  // 使用解引用操作符来访问 TOML 表的内容
            
            file.close();

            spdlog::info("Config saved to: {}", configPath_.toStdString());
            return true;
        } catch (const std::exception &e) {
            spdlog::error("Failed to save config: {}", e.what());
            return false;
        }
    }

    QString ConfigManager::getString(const QString &section, const QString &key,
                                     const QString &defaultValue) const {
        return QString::fromStdString(
          getValue<std::string>(section, key, defaultValue.toStdString()));
    }

    int ConfigManager::getInt(const QString &section, const QString &key,
                              int defaultValue) const {
        return static_cast<int>(getValue<int64_t>(section, key, defaultValue));
    }

    bool ConfigManager::getBool(const QString &section, const QString &key,
                                bool defaultValue) const {
        return getValue<bool>(section, key, defaultValue);
    }

    void ConfigManager::setString(const QString &section, const QString &key,
                                  const QString &value) {
        setValue<std::string>(section, key, value.toStdString());
    }

    void ConfigManager::setInt(const QString &section, const QString &key, int value) {
        setValue<int64_t>(section, key, value);
    }

    void ConfigManager::setBool(const QString &section, const QString &key, bool value) {
        setValue<bool>(section, key, value);
    }
}
