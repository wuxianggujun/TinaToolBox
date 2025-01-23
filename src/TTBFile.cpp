#include "TTBFile.hpp"
#include "TTBCrypto.hpp"
#include <iostream>
#include <sstream>

namespace TinaToolBox
{
    bool TTBFile::create(const std::string& filename,
                         const std::map<std::string, std::string>& config,
                         const std::string& script)
    {
        std::ofstream file(filename, std::ios::binary);
        if (!file)
        {
            return false;
        }

        // 准备文件头
        TTBHeader header;
        header.magic = TTB_MAGIC;
        header.version = TTB_VERSION;
        header.flags = 0; // 不加密

        // 先写入文件头（稍后更新实际的偏移量和大小）
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));

        // 写入配置区域
        header.configOffset = static_cast<uint32_t>(file.tellp());
        writeConfig(file, config, nullptr); // 不加密
        header.configSize = static_cast<uint32_t>(file.tellp()) - header.configOffset;

        // 写入脚本内容
        header.scriptOffset = static_cast<uint32_t>(file.tellp());
        file.write(script.c_str(), script.size());
        header.scriptSize = static_cast<uint32_t>(script.size());

        // 回到文件开头，更新文件头
        file.seekp(0);
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));

        return true;
    }

    bool TTBFile::createEncrypted(const std::string& filename,
                                  const std::map<std::string, std::string>& config,
                                  const std::string& script,
                                  const AESKey& key,
                                  EncryptionFlags flags)
    {
        // 检查数据大小
        constexpr size_t MAX_CONFIG_SIZE = 1024 * 1024; // 1MB
        constexpr size_t MAX_SCRIPT_SIZE = 10 * 1024 * 1024; // 10MB

        size_t totalConfigSize = 0;
        for (const auto& [k, v] : config)
        {
            totalConfigSize += k.size() + v.size() + sizeof(uint32_t) * 2;
        }
        totalConfigSize += sizeof(uint32_t); // 配置项数量

        if (totalConfigSize > MAX_CONFIG_SIZE)
        {
            std::cerr << "Config data too large" << std::endl;
            return false;
        }

        if (script.size() > MAX_SCRIPT_SIZE)
        {
            std::cerr << "Script data too large" << std::endl;
            return false;
        }

        std::ofstream file(filename, std::ios::binary);
        if (!file)
        {
            return false;
        }

        // 准备文件头
        TTBHeader header;
        header.magic = TTB_MAGIC;
        header.version = TTB_VERSION;
        header.flags = static_cast<uint16_t>(flags);
        header.iv = generateIV(); // 生成一个IV用于整个文件

        // 先写入文件头
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));

        // 写入配置区域
        header.configOffset = static_cast<uint32_t>(file.tellp());
        writeConfig(file, config,
                    (static_cast<uint16_t>(flags) & static_cast<uint16_t>(EncryptionFlags::ConfigEncrypted))
                        ? &key
                        : nullptr,
                    (static_cast<uint16_t>(flags) & static_cast<uint16_t>(EncryptionFlags::ConfigEncrypted))
                        ? &header.iv
                        : nullptr);
        header.configSize = static_cast<uint32_t>(file.tellp()) - header.configOffset;

        // 写入脚本内容
        header.scriptOffset = static_cast<uint32_t>(file.tellp());
        if (static_cast<uint16_t>(flags) & static_cast<uint16_t>(EncryptionFlags::ScriptEncrypted))
        {
            auto encrypted = Crypto::aesEncrypt(std::vector<uint8_t>(script.begin(), script.end()),
                                                key, header.iv); // 使用相同的IV
            file.write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());
            header.scriptSize = static_cast<uint32_t>(encrypted.size());
        }
        else
        {
            file.write(script.c_str(), script.size());
            header.scriptSize = static_cast<uint32_t>(script.size());
        }

        // 回到文件开头，更新文件头
        file.seekp(0);
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));

        return true;
    }

    void TTBFile::writeConfig(std::ofstream& file,
                              const std::map<std::string, std::string>& config,
                              const AESKey* key,
                              const AESIV* iv)
    {
        // 序列化配置
        std::stringstream ss;
        uint32_t count = static_cast<uint32_t>(config.size());
        ss.write(reinterpret_cast<const char*>(&count), sizeof(count));

        for (const auto& [k, v] : config)
        {
            uint32_t keySize = static_cast<uint32_t>(k.size());
            uint32_t valueSize = static_cast<uint32_t>(v.size());

            ss.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
            ss.write(k.c_str(), keySize);
            ss.write(reinterpret_cast<const char*>(&valueSize), sizeof(valueSize));
            ss.write(v.c_str(), valueSize);
        }

        std::string serialized = ss.str();

        if (key && iv)
        {
            // 如果提供了密钥和IV，加密配置
            auto encrypted = Crypto::aesEncrypt(
                std::vector<uint8_t>(serialized.begin(), serialized.end()),
                *key, *iv);
            file.write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());
        }
        else
        {
            // 否则直接写入
            file.write(serialized.c_str(), serialized.size());
        }
    }

    std::map<std::string, std::string> TTBFile::readConfig(std::istream& file,
                                                           uint32_t offset,
                                                           uint32_t size,
                                                           const AESKey* key,
                                                           const AESIV* iv)
    {
        file.seekg(offset);
        std::vector<uint8_t> data(size);
        file.read(reinterpret_cast<char*>(data.data()), size);

        if (key && iv)
        {
            // 如果提供了密钥和IV，解密数据
            data = Crypto::aesDecrypt(data, *key, *iv);
        }

        std::map<std::string, std::string> config;
        std::stringstream ss;
        ss.write(reinterpret_cast<const char*>(data.data()), data.size());

        uint32_t count;
        ss.read(reinterpret_cast<char*>(&count), sizeof(count));

        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t keySize, valueSize;
            ss.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));

            std::string key(keySize, '\0');
            ss.read(&key[0], keySize);

            ss.read(reinterpret_cast<char*>(&valueSize), sizeof(valueSize));

            std::string value(valueSize, '\0');
            ss.read(&value[0], valueSize);

            config[key] = value;
        }

        return config;
    }

    std::unique_ptr<TTBFile> TTBFile::load(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file)
        {
            return nullptr;
        }

        TTBHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));

        if (header.magic != TTB_MAGIC || header.version != TTB_VERSION)
        {
            return nullptr;
        }

        auto ttbFile = std::make_unique<TTBFile>();
        ttbFile->config_ = readConfig(file, header.configOffset, header.configSize);

        // 读取脚本内容
        file.seekg(header.scriptOffset);
        ttbFile->script_.resize(header.scriptSize);
        file.read(&ttbFile->script_[0], header.scriptSize);

        return ttbFile;
    }

    std::unique_ptr<TTBFile> TTBFile::loadEncrypted(const std::string& filename,
                                                    const AESKey& key)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file)
        {
            return nullptr;
        }

        TTBHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));

        if (header.magic != TTB_MAGIC || header.version != TTB_VERSION)
        {
            return nullptr;
        }

        auto ttbFile = std::make_unique<TTBFile>();

        // 读取并解密配置
        if (static_cast<uint16_t>(EncryptionFlags::ConfigEncrypted) & header.flags)
        {
            ttbFile->config_ = readConfig(file, header.configOffset, header.configSize, &key, &header.iv);
        }
        else
        {
            ttbFile->config_ = readConfig(file, header.configOffset, header.configSize);
        }

        // 读取并解密脚本
        file.seekg(header.scriptOffset);
        if (static_cast<uint16_t>(EncryptionFlags::ScriptEncrypted) & header.flags)
        {
            std::vector<uint8_t> encrypted(header.scriptSize);
            file.read(reinterpret_cast<char*>(encrypted.data()), header.scriptSize);
            auto decrypted = Crypto::aesDecrypt(encrypted, key, header.iv);
            ttbFile->script_ = std::string(decrypted.begin(), decrypted.end());
        }
        else
        {
            ttbFile->script_.resize(header.scriptSize);
            file.read(&ttbFile->script_[0], header.scriptSize);
        }

        return ttbFile;
    }

    std::unique_ptr<TTBFile> TTBFile::loadFromResource(const std::string& resourceName, const std::string& resourceType)
    {
        TTBResourceLoader loader;
        auto resourceData = loader.loadResource(resourceName, resourceType);
        if (resourceData.first == nullptr)
        {
            return nullptr;
        }
        return loadFromMemory(resourceData.first, resourceData.second);
    }

    std::unique_ptr<TTBFile> TTBFile::loadEncryptedFromResource(const std::string& resourceName,
                                                                const std::string& resourceType, const AESKey& key)
    {
        TTBResourceLoader loader;
        auto resourceData = loader.loadResource(resourceName, resourceType);
        if (resourceData.first == nullptr)
        {
            return nullptr;
        }
        return loadEncryptedFromMemory(resourceData.first, resourceData.second, key);
    }

    std::unique_ptr<TTBFile> TTBFile::loadFromMemory(const char* data, size_t size)
    {
        std::stringstream memoryStream;
        memoryStream.write(data, size);
        if (!memoryStream) return nullptr;

        TTBHeader header;
        memoryStream.read(reinterpret_cast<char*>(&header), sizeof(header));

        if (header.magic != TTB_MAGIC || header.version != TTB_VERSION)
        {
            return nullptr;
        }

        auto ttbFile = std::make_unique<TTBFile>();
        ttbFile->config_ = readConfig(memoryStream, header.configOffset, header.configSize);

        memoryStream.seekg(header.scriptOffset);
        ttbFile->script_.resize(header.scriptSize);
        memoryStream.read(&ttbFile->script_[0], header.scriptSize);

        return ttbFile;
    }

    std::unique_ptr<TTBFile> TTBFile::loadEncryptedFromMemory(const char* data, size_t size, const AESKey& key)
    {
        std::stringstream memoryStream;
        memoryStream.write(data, size);
        if (!memoryStream) return nullptr;

        TTBHeader header;
        memoryStream.read(reinterpret_cast<char*>(&header), sizeof(header));

        if (header.magic != TTB_MAGIC || header.version != TTB_VERSION)
        {
            return nullptr;
        }

        auto ttbFile = std::make_unique<TTBFile>();

        if (static_cast<uint16_t>(EncryptionFlags::ConfigEncrypted) & header.flags)
        {
            ttbFile->config_ = readConfig(memoryStream, header.configOffset, header.configSize, &key, &header.iv);
        }
        else
        {
            ttbFile->config_ = readConfig(memoryStream, header.configOffset, header.configSize);
        }

        memoryStream.seekg(header.scriptOffset);
        if (static_cast<uint16_t>(EncryptionFlags::ScriptEncrypted) & header.flags)
        {
            std::vector<uint8_t> encrypted(header.scriptSize);
            memoryStream.read(reinterpret_cast<char*>(encrypted.data()), header.scriptSize);
            auto decrypted = Crypto::aesDecrypt(encrypted, key, header.iv);
            ttbFile->script_ = std::string(decrypted.begin(), decrypted.end());
        }
        else
        {
            ttbFile->script_.resize(header.scriptSize);
            memoryStream.read(&ttbFile->script_[0], header.scriptSize);
        }

        return ttbFile;
    }

    std::string TTBFile::getConfigValue(const std::string& key, const std::string& defaultValue) const
    {
        auto it = config_.find(key);
        return it != config_.end() ? it->second : defaultValue;
    }

    AESKey TTBFile::generateKey()
    {
        return Crypto::generateKey();
    }

    AESIV TTBFile::generateIV()
    {
        return Crypto::generateIV();
    }

    bool TTBFile::isEncrypted(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            return false;
        }

        TTBHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        
        if (header.magic != TTB_MAGIC || header.version != TTB_VERSION) {
            return false;
        }

        return (header.flags & static_cast<uint16_t>(EncryptionFlags::AllEncrypted)) != 0;
    }

    bool TTBFile::isEncryptedFromMemory(const char* data, size_t size)
    {
        if (!data || size < sizeof(TTBHeader)) {
            return false;
        }

        const TTBHeader* header = reinterpret_cast<const TTBHeader*>(data);
        
        if (header->magic != TTB_MAGIC || header->version != TTB_VERSION) {
            return false;
        }

        return (header->flags & static_cast<uint16_t>(EncryptionFlags::AllEncrypted)) != 0;
    }
} // namespace TinaToolBox
