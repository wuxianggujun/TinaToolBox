#include "TTBResourceHandle.hpp"
#include <spdlog/spdlog.h>

namespace TinaToolBox {

TTBResourceHandle::TTBResourceHandle(const std::string& path)
    : handle_(nullptr)
    , committed_(false) {
    // 将路径转换为宽字符
    auto widePath = std::filesystem::path(path).wstring();
    
    // 开始资源更新
    handle_ = BeginUpdateResourceW(
        reinterpret_cast<LPCWSTR>(widePath.c_str()),
        FALSE);
        
    if (!handle_) {
        DWORD error = GetLastError();
        throw std::runtime_error(
            "Failed to begin resource update. Error code: " + 
            std::to_string(error));
    }
    
    spdlog::debug("Opened resource handle for file: {}", path);
}

TTBResourceHandle::~TTBResourceHandle() {
    if (handle_ && !committed_) {
        // 如果没有提交更改，则取消更改
        EndUpdateResource(handle_, TRUE);
        spdlog::debug("Cancelled resource updates");
    }
}

TTBResourceHandle::TTBResourceHandle(TTBResourceHandle&& other) noexcept
    : handle_(other.handle_)
    , committed_(other.committed_) {
    other.handle_ = nullptr;
    other.committed_ = false;
}

TTBResourceHandle& TTBResourceHandle::operator=(TTBResourceHandle&& other) noexcept {
    if (this != &other) {
        if (handle_ && !committed_) {
            EndUpdateResource(handle_, TRUE);
        }
        handle_ = other.handle_;
        committed_ = other.committed_;
        other.handle_ = nullptr;
        other.committed_ = false;
    }
    return *this;
}

void TTBResourceHandle::commit() {
    if (!handle_) {
        throw std::runtime_error("Invalid resource handle");
    }
    
    if (committed_) {
        throw std::runtime_error("Resource already committed");
    }
    
    if (!EndUpdateResource(handle_, FALSE)) {
        DWORD error = GetLastError();
        throw std::runtime_error(
            "Failed to commit resource update. Error code: " + 
            std::to_string(error));
    }
    
    spdlog::debug("Committed resource updates");
    handle_ = nullptr;
    committed_ = true;
}

void TTBResourceHandle::cancel() {
    if (!handle_) {
        throw std::runtime_error("Invalid resource handle");
    }
    
    if (committed_) {
        throw std::runtime_error("Resource already committed");
    }
    
    if (!EndUpdateResource(handle_, TRUE)) {
        DWORD error = GetLastError();
        throw std::runtime_error(
            "Failed to cancel resource update. Error code: " + 
            std::to_string(error));
    }
    
    spdlog::debug("Cancelled resource updates");
    handle_ = nullptr;
    committed_ = false;
}

} // namespace TinaToolBox 