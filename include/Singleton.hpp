#pragma once

#include "NonCopyable.hpp"
#include <mutex>
#include <memory>

namespace TinaToolBox {
    template<typename T>
    class Singleton : public NonCopyable {
    public:
        static T &getInstance() {
            static T instance;
            return instance;
        }

        virtual void initialize() {
        }

        virtual void shutdown() {
        }

    protected:
        Singleton() = default;

        virtual ~Singleton() = default;
    };
}
