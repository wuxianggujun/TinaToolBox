//
// Created by wuxianggujun on 2024/12/17.
//

#include "UIConfig.hpp"
#include "ConfigManager.hpp"

namespace TinaToolBox {
    void UIConfig::initialize() {
        Singleton<UIConfig>::initialize();
    }

    void UIConfig::shutdown() {
        Singleton<UIConfig>::shutdown();
    }

    int UIConfig::cornerRadius() const {
        return ConfigManager::getInstance().getInt("window", "corner_radius", 10);
    }
} // TinaToolBox
