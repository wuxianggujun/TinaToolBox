#pragma once

namespace TinaToolBox {
    class NonCopyable {
    protected:
        NonCopyable() = default;

        ~NonCopyable() = default;

    private:
        NonCopyable(const NonCopyable &) = delete;

        NonCopyable &operator=(const NonCopyable &) = delete;
    };
}
