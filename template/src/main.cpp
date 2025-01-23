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
#include "TTBCrypto.hpp"

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
        zs.zalloc = Z_NULL;
        zs.zfree = Z_NULL;
        zs.opaque = Z_NULL;
        
        // 使用inflateInit2来匹配deflateInit2的设置
        if (inflateInit2(&zs, 15 + 16) != Z_OK) {
            std::cerr << "Failed to initialize zlib" << std::endl;
            return 1;
        }

        zs.next_in = compressedData.data();
        zs.avail_in = static_cast<uInt>(compressedData.size());
        zs.next_out = uncompressedData.data();
        zs.avail_out = static_cast<uInt>(uncompressedData.size());

        // 执行解压
        int ret = inflate(&zs, Z_FINISH);
        if (ret != Z_STREAM_END) {
            std::cerr << "Failed to decompress TTB data (ret=" << ret << ")" << std::endl;
            std::cerr << "zlib error: " << (zs.msg ? zs.msg : "unknown error") << std::endl;
            std::cerr << "Input size: " << compressedData.size() << std::endl;
            std::cerr << "Output buffer size: " << uncompressedData.size() << std::endl;
            std::cerr << "Bytes written: " << zs.total_out << std::endl;
            
            // 打印前几个字节用于调试
            std::cerr << "First few bytes of compressed data: ";
            for (size_t i = 0; i < std::min(size_t(16), compressedData.size()); ++i) {
                std::cerr << std::hex << (int)compressedData[i] << " ";
            }
            std::cerr << std::dec << std::endl;
            
            inflateEnd(&zs);
            return 1;
        }

        inflateEnd(&zs);
        uncompressedData.resize(zs.total_out);
        
        std::cout << "Decompressed size: " << uncompressedData.size() << " bytes" << std::endl;

        // 3. 创建临时文件
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

        // 4. 检查文件是否加密
        bool isEncrypted = TTBFile::isEncrypted(tempPath.string());
        std::cout << "TTB file " << (isEncrypted ? "is" : "is not") << " encrypted" << std::endl;

        // 5. 根据加密状态加载TTB文件
        std::unique_ptr<TTBFile> ttbFile;
        try {
            if (isEncrypted) {
                // 使用默认密钥
                AESKey defaultKey = {
                    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
                };
                std::cout << "Loading encrypted TTB file with default key..." << std::endl;
                ttbFile = TTBFile::loadEncrypted(tempPath.string(), defaultKey);
            } else {
                std::cout << "Loading unencrypted TTB file..." << std::endl;
                ttbFile = TTBFile::load(tempPath.string());
            }

            if (!ttbFile) {
                std::cerr << "Failed to load TTB file" << std::endl;
                std::filesystem::remove(tempPath);
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to load TTB file: " << e.what() << std::endl;
            std::filesystem::remove(tempPath);
            return 1;
        }

        // 6. 执行TTB文件
        TTBScriptEngine engine;
        
        // 设置进度回调
        engine.setProgressCallback([](const std::string& message, int progress) {
            std::cout << "[" << progress << "%] " << message << std::endl;
        });

        // 使用默认密钥
        AESKey defaultKey = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
            0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
        };

        auto result = engine.executeScript(tempPath.string(), defaultKey, ttbFile.get());

        // 7. 清理临时文件
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