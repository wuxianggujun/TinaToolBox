//
// Created by wuxianggujun on 2025/1/22.
//

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace TinaToolBox
{
    class TTBResourceLoader
    {
    public:
        TTBResourceLoader() = default;
        ~TTBResourceLoader() = default;

        std::pair<const char*, size_t> loadResource(const std::string& resourceName, const std::string& resourceType);

    private:
#ifdef _WIN32
        // Windows 平台资源加载实现
        std::pair<const char*, size_t> loadResourceWindows(const std::string& resourceName,
                                                           const std::string& resourceType);
#else
        // 其他平台资源加载实现 (可以添加 Linux, macOS 等平台的实现)
        std::pair<const char*, size_t> loadResourceGeneric(const std::string& resourceName, const std::string& resourceType);
#endif
    };
} // TinaToolBox
