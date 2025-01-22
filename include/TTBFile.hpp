#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <array>

namespace TinaToolBox {

    // TTB文件格式的魔数，用于标识文件类型
    constexpr uint32_t TTB_MAGIC = 0x00425454;  // "\0TTB" in ASCII (0x54='T', 0x42='B')
    constexpr uint16_t TTB_VERSION = 0x0100;    // 版本1.0 (主版本号.次版本号)

    // 加密相关常量
    constexpr size_t AES_KEY_SIZE = 32;  // AES-256
    constexpr size_t AES_IV_SIZE = 16;   // AES IV size
    using AESKey = std::array<uint8_t, AES_KEY_SIZE>;
    using AESIV = std::array<uint8_t, AES_IV_SIZE>;

    // 加密标志位
    enum class EncryptionFlags : uint16_t {
        None = 0x0000,
        ConfigEncrypted = 0x0001,
        ScriptEncrypted = 0x0002,
        AllEncrypted = ConfigEncrypted | ScriptEncrypted
    };

    // TTB文件头结构
    struct TTBHeader {
        uint32_t magic;          // 魔数，用于标识文件类型
        uint16_t version;        // 文件格式版本 (2字节)
        uint16_t flags;          // 标志位（包含加密信息）
        uint32_t configOffset;   // 配置区域的偏移量
        uint32_t configSize;     // 配置区域的大小
        uint32_t scriptOffset;   // 脚本区域的偏移量
        uint32_t scriptSize;     // 脚本区域的大小
        AESIV iv;               // 初始化向量
    };

    class TTBFile {
    public:
        // 创建新的TTB文件（不加密）
        static bool create(const std::string& filename,
                         const std::map<std::string, std::string>& config,
                         const std::string& script);

        // 创建新的加密TTB文件
        static bool createEncrypted(const std::string& filename,
                                  const std::map<std::string, std::string>& config,
                                  const std::string& script,
                                  const AESKey& key,
                                  EncryptionFlags flags = EncryptionFlags::AllEncrypted);

        // 读取TTB文件（不加密）
        static std::unique_ptr<TTBFile> load(const std::string& filename);

        // 读取加密的TTB文件
        static std::unique_ptr<TTBFile> loadEncrypted(const std::string& filename,
                                                     const AESKey& key);

        // 获取配置
        const std::map<std::string, std::string>& getConfig() const { return config_; }

        // 获取脚本内容
        const std::string& getScript() const { return script_; }

        // 获取特定配置项
        std::string getConfigValue(const std::string& key, const std::string& defaultValue = "") const;

        // 生成随机密钥
        static AESKey generateKey();

        // 允许创建实例
        TTBFile() = default;
        
    private:
        // 写入配置区域
        static void writeConfig(std::ofstream& file, 
                              const std::map<std::string, std::string>& config,
                              const AESKey* key = nullptr,
                              const AESIV* iv = nullptr);
        
        // 读取配置区域
        static std::map<std::string, std::string> readConfig(std::ifstream& file, 
                                                           uint32_t offset, 
                                                           uint32_t size,
                                                           const AESKey* key = nullptr,
                                                           const AESIV* iv = nullptr);

        // 加密数据
        static std::vector<uint8_t> encryptData(const std::vector<uint8_t>& data,
                                              const AESKey& key,
                                              const AESIV& iv);

        // 解密数据
        static std::vector<uint8_t> decryptData(const std::vector<uint8_t>& data,
                                              const AESKey& key,
                                              const AESIV& iv);

        // 生成随机IV
        static AESIV generateIV();

        std::map<std::string, std::string> config_;
        std::string script_;
    };

    // 定义按位运算符
    inline EncryptionFlags operator|(EncryptionFlags a, EncryptionFlags b) {
        return static_cast<EncryptionFlags>(
            static_cast<uint16_t>(a) | static_cast<uint16_t>(b)
        );
    }

    inline EncryptionFlags operator&(EncryptionFlags a, EncryptionFlags b) {
        return static_cast<EncryptionFlags>(
            static_cast<uint16_t>(a) & static_cast<uint16_t>(b)
        );
    }

} // namespace TinaToolBox 