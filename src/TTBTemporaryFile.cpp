#include "TTBTemporaryFile.hpp"
#include <random>
#include <spdlog/spdlog.h>

namespace TinaToolBox {

TTBTemporaryFile::TTBTemporaryFile(const std::string& prefix)
    : isValid_(true) {
    // 生成随机数作为文件名的一部分
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);
    
    // 构造临时文件路径
    path_ = std::filesystem::temp_directory_path() / 
            (prefix + std::to_string(dis(gen)));
            
    spdlog::debug("Created temporary file: {}", path_.string());
}

TTBTemporaryFile::~TTBTemporaryFile() {
    try {
        if (isValid_) {
            remove();
        }
    } catch (const std::exception& e) {
        // 在析构函数中不抛出异常，只记录错误
        spdlog::error("Failed to remove temporary file: {}, error: {}", 
                     path_.string(), e.what());
    }
}

TTBTemporaryFile::TTBTemporaryFile(TTBTemporaryFile&& other) noexcept
    : path_(std::move(other.path_))
    , isValid_(other.isValid_) {
    other.isValid_ = false;
}

TTBTemporaryFile& TTBTemporaryFile::operator=(TTBTemporaryFile&& other) noexcept {
    if (this != &other) {
        if (isValid_) {
            try {
                remove();
            } catch (...) {
                // 忽略错误
            }
        }
        path_ = std::move(other.path_);
        isValid_ = other.isValid_;
        other.isValid_ = false;
    }
    return *this;
}

void TTBTemporaryFile::remove() {
    if (isValid_) {
        if (std::filesystem::exists(path_)) {
            std::filesystem::remove(path_);
            spdlog::debug("Removed temporary file: {}", path_.string());
        }
        isValid_ = false;
    }
}

} // namespace TinaToolBox 