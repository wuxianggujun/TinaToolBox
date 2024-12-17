//
// Created by wuxianggujun on 2024/12/17.
//
#pragma once
#include "Singleton.hpp"

namespace TinaToolBox {
    class UIConfig : public Singleton<UIConfig>{
    public:
        void initialize() override;

        void shutdown() override;

        int cornerRadius() const;
    };
} // TinaToolBox
