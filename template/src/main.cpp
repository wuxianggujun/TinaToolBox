#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <zlib.h>
#include "TTBScriptEngine.hpp"
#include "TTBFile.hpp"
#include "TTBResourceExtractor.hpp"

using namespace TinaToolBox;

int main(int argc, char* argv[]) {
    try {
        // 1. 从资源段提取压缩的TTB数据
        TTBResourceExtractor extractor;
        auto compressedData = extractor.extractTTBData();
        if (compressedData.empty()) {
            std::cerr << "No TTB data found in resources" << std::endl;
            return 1;
        }

        // 2. 解压TTB数据
        z_stream zs;
        std::vector<uint8_t> uncompressedData;
        memset(&zs, 0, sizeof(zs));
        if (inflateInit(&zs) != Z_OK) {
            std::cerr << "Failed to initialize zlib" << std::endl;
            return 1;
        }

        // 设置输入
        zs.next_in = compressedData.data();
        zs.avail_in = static_cast<uInt>(compressedData.size());

        // 分配解压缓冲区（假设解压后大小不超过压缩数据的4倍）
        uncompressedData.resize(compressedData.size() * 4);
        zs.next_out = uncompressedData.data();
        zs.avail_out = static_cast<uInt>(uncompressedData.size());

        // 执行解压
        int ret = inflate(&zs, Z_FINISH);
        inflateEnd(&zs);

        if (ret != Z_STREAM_END) {
            std::cerr << "Failed to decompress TTB data" << std::endl;
            return 1;
        }

        // 调整到实际大小
        uncompressedData.resize(zs.total_out);

        // 3. 创建临时文件
        std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "temp.ttb";
        {
            std::ofstream tempFile(tempPath, std::ios::binary);
            if (!tempFile) {
                std::cerr << "Failed to create temporary file" << std::endl;
                return 1;
            }
            tempFile.write(reinterpret_cast<const char*>(uncompressedData.data()), uncompressedData.size());
        }

        // 4. 执行TTB文件
        TTBScriptEngine engine;
        auto result = engine.executeScript(tempPath.string());

        // 5. 清理临时文件
        std::filesystem::remove(tempPath);

        if (result != TTBScriptEngine::Error::SUCCESS) {
            std::cerr << "Failed to execute TTB script: " << engine.getLastError() << std::endl;
            return 1;
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 