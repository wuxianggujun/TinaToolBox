#include "TTBCrypto.hpp"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <stdexcept>

namespace TinaToolBox {
namespace Crypto {

    std::vector<uint8_t> aesEncrypt(const std::vector<uint8_t>& data,
                                   const AESKey& key,
                                   const AESIV& iv) {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }

        std::vector<uint8_t> encrypted;
        encrypted.resize(data.size() + EVP_MAX_BLOCK_LENGTH);
        int outlen1 = 0;
        int outlen2 = 0;

        if (EVP_EncryptUpdate(ctx, encrypted.data(), &outlen1, data.data(), static_cast<int>(data.size())) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to encrypt data");
        }

        if (EVP_EncryptFinal_ex(ctx, encrypted.data() + outlen1, &outlen2) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize encryption");
        }

        EVP_CIPHER_CTX_free(ctx);
        encrypted.resize(outlen1 + outlen2);
        return encrypted;
    }

    std::vector<uint8_t> aesDecrypt(const std::vector<uint8_t>& data,
                                   const AESKey& key,
                                   const AESIV& iv) {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize decryption");
        }

        std::vector<uint8_t> decrypted;
        decrypted.resize(data.size());
        int outlen1 = 0;
        int outlen2 = 0;

        if (EVP_DecryptUpdate(ctx, decrypted.data(), &outlen1, data.data(), static_cast<int>(data.size())) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to decrypt data");
        }

        if (EVP_DecryptFinal_ex(ctx, decrypted.data() + outlen1, &outlen2) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize decryption");
        }

        EVP_CIPHER_CTX_free(ctx);
        decrypted.resize(outlen1 + outlen2);
        return decrypted;
    }

    AESKey generateKey() {
        AESKey key;
        if (RAND_bytes(key.data(), static_cast<int>(key.size())) != 1) {
            throw std::runtime_error("Failed to generate random key");
        }
        return key;
    }

    AESIV generateIV() {
        AESIV iv;
        if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1) {
            throw std::runtime_error("Failed to generate random IV");
        }
        return iv;
    }

} // namespace Crypto
} // namespace TinaToolBox 