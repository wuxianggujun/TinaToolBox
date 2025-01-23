#pragma once

#include <string>
#include <windows.h>
#include <filesystem>
#include <stdexcept>

namespace TinaToolBox {

class TTBResourceHandle {
public:
    explicit TTBResourceHandle(const std::string& path);
    ~TTBResourceHandle();

    // 禁止拷贝
    TTBResourceHandle(const TTBResourceHandle&) = delete;
    TTBResourceHandle& operator=(const TTBResourceHandle&) = delete;

    // 允许移动
    TTBResourceHandle(TTBResourceHandle&&) noexcept;
    TTBResourceHandle& operator=(TTBResourceHandle&&) noexcept;

    // 获取原始句柄
    HANDLE get() const { return handle_; }

    // 提交更改
    void commit();

    // 取消更改
    void cancel();

    // 检查句柄是否有效
    bool isValid() const { return handle_ != nullptr; }

private:
    HANDLE handle_;
    bool committed_;
};

} // namespace TinaToolBox 