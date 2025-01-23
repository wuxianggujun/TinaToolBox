#pragma once

#include <string>
#include <filesystem>

namespace TinaToolBox {

class TTBTemporaryFile {
public:
    explicit TTBTemporaryFile(const std::string& prefix = "ttb_");
    ~TTBTemporaryFile();

    // 禁止拷贝
    TTBTemporaryFile(const TTBTemporaryFile&) = delete;
    TTBTemporaryFile& operator=(const TTBTemporaryFile&) = delete;

    // 允许移动
    TTBTemporaryFile(TTBTemporaryFile&&) noexcept;
    TTBTemporaryFile& operator=(TTBTemporaryFile&&) noexcept;

    // 获取临时文件路径
    const std::filesystem::path& path() const { return path_; }

    // 显式删除文件
    void remove();

private:
    std::filesystem::path path_;
    bool isValid_;
};

} // namespace TinaToolBox 