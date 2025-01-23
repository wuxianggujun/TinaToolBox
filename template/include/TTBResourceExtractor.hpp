#pragma once
#include <vector>
#include <string>
#include <windows.h>

namespace TinaToolBox {

class TTBResourceExtractor {
public:
    // 从资源段提取TTB数据
    std::vector<uint8_t> extractTTBData() {
        HMODULE hModule = GetModuleHandle(NULL);
        if (!hModule) {
            return {};
        }

        // 查找TTB资源
        HRSRC hRes = FindResource(hModule, L"TTB_DATA", RT_RCDATA);
        if (!hRes) {
            return {};
        }

        // 获取资源大小
        DWORD size = SizeofResource(hModule, hRes);
        if (size == 0) {
            return {};
        }

        // 加载资源
        HGLOBAL hGlobal = LoadResource(hModule, hRes);
        if (!hGlobal) {
            return {};
        }

        // 锁定资源
        void* data = LockResource(hGlobal);
        if (!data) {
            return {};
        }

        // 复制数据
        std::vector<uint8_t> buffer(static_cast<uint8_t*>(data),
                                  static_cast<uint8_t*>(data) + size);

        return buffer;
    }
};
} 