#pragma once
#include <vector>
#include <string>
#include <windows.h>
#include <psapi.h>
#include <cstring>
#include <stdexcept>

namespace TinaToolBox {

class TTBResourceExtractor {
public:
    // 从可执行文件中提取TTB数据
    std::vector<uint8_t> extractTTBData() {
        // 获取当前模块句柄
        HMODULE hModule = GetModuleHandle(NULL);
        if (!hModule) {
            throw std::runtime_error("Failed to get module handle");
        }

        // 查找TTB资源
        HRSRC hRes = FindResourceW(hModule, MAKEINTRESOURCEW(1), L"TTB_DATA");
        if (!hRes) {
            DWORD error = GetLastError();
            throw std::runtime_error("Failed to find TTB_DATA resource. Error code: " + std::to_string(error));
        }

        // 获取资源大小
        DWORD size = SizeofResource(hModule, hRes);
        if (size == 0) {
            DWORD error = GetLastError();
            throw std::runtime_error("Resource size is 0. Error code: " + std::to_string(error));
        }

        // 加载资源
        HGLOBAL hGlobal = LoadResource(hModule, hRes);
        if (!hGlobal) {
            DWORD error = GetLastError();
            throw std::runtime_error("Failed to load resource. Error code: " + std::to_string(error));
        }

        // 锁定资源内存
        const void* data = LockResource(hGlobal);
        if (!data) {
            DWORD error = GetLastError();
            throw std::runtime_error("Failed to lock resource. Error code: " + std::to_string(error));
        }

        // 复制资源数据
        std::vector<uint8_t> buffer(size);
        memcpy(buffer.data(), data, size);

        // 验证数据大小
        if (buffer.size() != size) {
            throw std::runtime_error("Buffer size mismatch. Expected: " + std::to_string(size) + 
                                   ", Got: " + std::to_string(buffer.size()));
        }

        return buffer;
    }

    // 添加一个调试方法来获取资源信息
    void debugResourceInfo() {
        try {
            HMODULE hModule = GetModuleHandle(NULL);
            if (!hModule) {
                throw std::runtime_error("Failed to get module handle");
            }

            // 枚举所有资源类型
            EnumResourceTypesW(hModule, 
                [](HMODULE hModule, LPWSTR lpszType, LONG_PTR lParam) -> BOOL {
                    wchar_t typeName[256] = L"";
                    if (IS_INTRESOURCE(lpszType)) {
                        swprintf_s(typeName, L"#%d", (int)(ULONG_PTR)lpszType);
                    } else {
                        wcscpy_s(typeName, lpszType);
                    }
                    
                    // 枚举该类型的所有资源
                    EnumResourceNamesW(hModule, lpszType,
                        [](HMODULE hModule, LPCWSTR lpszType, LPWSTR lpszName, LONG_PTR lParam) -> BOOL {
                            HRSRC hRes = FindResourceW(hModule, lpszName, lpszType);
                            if (hRes) {
                                DWORD size = SizeofResource(hModule, hRes);
                                char msg[512];
                                sprintf_s(msg, "Found resource - Type: %ls, Size: %lu bytes\n", 
                                        (IS_INTRESOURCE(lpszType) ? L"#?" : lpszType), size);
                                OutputDebugStringA(msg);
                            }
                            return TRUE;
                        }, 0);
                    return TRUE;
                }, 0);

        } catch (const std::exception& e) {
            OutputDebugStringA(e.what());
        }
    }
};
} 