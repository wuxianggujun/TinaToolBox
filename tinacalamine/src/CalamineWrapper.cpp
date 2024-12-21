#include "tinacalamine/include/CalamineWrapper.hpp"
#include "tinacalamine/src/lib.rs.h"
#include <iostream>
#include "calamine/calamine.hpp"

using namespace calamine;

class CalamineWrapper::Impl {
public:
    Impl() {
        // 初始化 Calamine 相关代码
        std::cout << "CalamineWrapper::Impl created" << std::endl;
    }

    ~Impl() {
        std::cout << "CalamineWrapper::Impl destroyed" << std::endl;
    }

    int read_a_value() const{
        // 使用 Calamine 读取值的代码
        return 42; // 示例返回值
    }
};

CalamineWrapper::CalamineWrapper() : pImpl(std::make_unique<Impl>()) {}

CalamineWrapper::~CalamineWrapper() = default;

int CalamineWrapper::read_a_value() const{
    return pImpl->read_a_value();
}

// Rust 接口函数

std::unique_ptr<CalamineWrapper> new_calamine() {
    return std::make_unique<CalamineWrapper>();
}