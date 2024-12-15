#pragma once

#include "NonCopyable.hpp"

namespace TinaToolBox {
    template<typename T>
    class Singleton : public NonCopyable {
    public:
        static T &getInstance() {
            static T instance;
            return instance;
        }

    protected:
        Singleton() = default;

        ~Singleton() = default;
    };
}
