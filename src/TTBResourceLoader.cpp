//
// Created by wuxianggujun on 2025/1/22.
//

#include "TTBResourceLoader.hpp"
#include <stdexcept>
#include <iostream> // 引入 iostream 以便输出错误信息

#ifdef _WIN32
#include <Windows.h>
#endif

namespace TinaToolBox
{
    std::pair<const char*, size_t> TTBResourceLoader::loadResource(const std::string& resourceName,
                                                                   const std::string& resourceType)
    {
#ifdef _WIN32
        return loadResourceWindows(resourceName, resourceType);
#else
        return loadResourceGeneric(resourceName, resourceType);
#endif
    }
#ifdef _WIN32
    std::pair<const char*, size_t> TTBResourceLoader::loadResourceWindows(const std::string& resourceName,
                                                                          const std::string& resourceType)
    {
        HRSRC hRes = FindResource(nullptr, reinterpret_cast<LPCWSTR>(resourceName.c_str()),
                                  reinterpret_cast<LPCWSTR>(resourceType.c_str()));
        if (!hRes)
        {
            std::cerr << "Error: FindResource failed for resource " << resourceName << " of type " << resourceType <<
                std::endl;
            return {nullptr, 0};
        }

        HGLOBAL hResLoad = LoadResource(nullptr, hRes);
        if (!hResLoad)
            std::cerr << "Error: LoadResource failed for resource " << resourceName << " of type " << resourceType <<
                std::endl;
        return {nullptr, 0};

        const char* pResData = static_cast<const char*>(LockResource(hResLoad));
        if (!pResData)
            std::cerr << "Error: LockResource failed for resource " << resourceName << " of type " << resourceType <<
                std::endl;
        return {nullptr, 0};

        DWORD resSize = SizeofResource(nullptr, hRes);
        if (!resSize)
        {
            std::cerr << "Error: SizeofResource failed for resource " << resourceName << " of type " << resourceType <<
                std::endl;
            return {nullptr, 0};
        }
        return {pResData, resSize};
    }

#else
    std::pair<const char*, size_t> TTBResourceLoader::loadResourceGeneric(const std::string& resourceName, const std::string& resourceType) {
        // 在非 Windows 平台上，如果没有平台特定的资源加载机制，
        // 你可能需要实现一个通用的资源加载方法，或者返回错误。
        // 例如，可以始终返回 nullptr 和 0 大小，表示资源加载失败。
        std::cerr << "Warning: Resource loading not implemented for this platform. Resource: " << resourceName << ", type: " << resourceType << std::endl;
        (void)resourceName; // 避免未使用参数警告
        (void)resourceType;
        return {nullptr, 0};
        // 或者抛出异常: throw std::runtime_error("Resource loading not implemented for this platform.");
    }
#endif
} // TinaToolBox
