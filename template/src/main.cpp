#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include <zlib.h>
#include "TTBScriptEngine.hpp"
#include "TTBFile.hpp"
#include "TTBResourceExtractor.hpp"

using namespace TinaToolBox;

int main(int argc, char* argv[]) {
    try {
        // 1. 从资源段提取压缩的TTB数据
        TTBResourceExtractor extractor;
        
        // 添加调试信息
        extractor.debugResourceInfo();
        
        auto compressedData = extractor.extractTTBData();
        if (compressedData.empty()) {
            std::cerr << "No TTB data found in resources" << std::endl;
            return 1;
        }
        
        std::cout << "Extracted TTB data size: " << compressedData.size() << " bytes" << std::endl;

        // 2. 解压TTB数据
        std::vector<uint8_t> uncompressedData;
        uncompressedData.resize(compressedData.size() * 10); // 增加缓冲区大小
        
        z_stream zs = {0};
        zs.next_in = compressedData.data();
        zs.avail_in = static_cast<uInt>(compressedData.size());
        zs.next_out = uncompressedData.data();
        zs.avail_out = static_cast<uInt>(uncompressedData.size());

        if (inflateInit(&zs) != Z_OK) {
            std::cerr << "Failed to initialize zlib" << std::endl;
            return 1;
        }

        // 执行解压
        int ret = inflate(&zs, Z_FINISH);
        if (ret != Z_STREAM_END) {
            std::cerr << "Failed to decompress TTB data (ret=" << ret << ")" << std::endl;
            // 打印更多调试信息
            std::cerr << "zlib error: " << (zs.msg ? zs.msg : "unknown error") << std::endl;
            std::cerr << "Input size: " << compressedData.size() << std::endl;
            std::cerr << "Output buffer size: " << uncompressedData.size() << std::endl;
            std::cerr << "Bytes written: " << zs.total_out << std::endl;
            inflateEnd(&zs);
            return 1;
        }

        inflateEnd(&zs);
        uncompressedData.resize(zs.total_out);
        
        std::cout << "Decompressed size: " << uncompressedData.size() << " bytes" << std::endl;

        // 3. 验证解压后的数据
        if (uncompressedData.size() < 4) {
            std::cerr << "Decompressed data too small" << std::endl;
            return 1;
        }

        // 检查TTB文件头
        if (memcmp(uncompressedData.data(), "TTB", 3) != 0) {
            std::cerr << "Invalid TTB file header" << std::endl;
            return 1;
        }

        // 4. 创建临时文件
        std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "temp.ttb";
        {
            std::ofstream tempFile(tempPath, std::ios::binary);
            if (!tempFile) {
                std::cerr << "Failed to create temporary file: " << tempPath.string() << std::endl;
                return 1;
            }
            tempFile.write(reinterpret_cast<const char*>(uncompressedData.data()), uncompressedData.size());
            tempFile.close();
            
            std::cout << "Created temporary file: " << tempPath.string() << std::endl;
        }

        // 5. 执行TTB文件
        TTBScriptEngine engine;
        
        // 设置进度回调
        engine.setProgressCallback([](const std::string& message, int progress) {
            std::cout << "[" << progress << "%] " << message << std::endl;
        });

        auto result = engine.executeScript(tempPath.string());

        // 6. 清理临时文件
        std::filesystem::remove(tempPath);

        if (result != TTBScriptEngine::Error::SUCCESS) {
            std::cerr << "Failed to execute TTB script: " << engine.getLastError() << std::endl;
            return 1;
        }

        std::cout << "Script executed successfully" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 