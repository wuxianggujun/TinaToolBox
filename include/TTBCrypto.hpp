#pragma once
#include <vector>
#include <array>
#include <random>
#include <cstdint>

namespace TinaToolBox {
namespace Crypto {

    constexpr size_t AES_KEY_SIZE = 32;  // AES-256
    constexpr size_t AES_IV_SIZE = 16;   // AES IV size
    using AESKey = std::array<uint8_t, AES_KEY_SIZE>;
    using AESIV = std::array<uint8_t, AES_IV_SIZE>;

    // 加密数据
    std::vector<uint8_t> aesEncrypt(const std::vector<uint8_t>& data,
                                   const AESKey& key,
                                   const AESIV& iv);

    // 解密数据
    std::vector<uint8_t> aesDecrypt(const std::vector<uint8_t>& data,
                                   const AESKey& key,
                                   const AESIV& iv);

    // 生成随机密钥
    AESKey generateKey();

    // 生成随机IV
    AESIV generateIV();

} // namespace Crypto
} // namespace TinaToolBox 