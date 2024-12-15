#pragma once

#include "NonCopyable.hpp"
#include <mutex>
#include <memory>

namespace TinaToolBox {
    template<typename T>
    class Singleton : public NonCopyable {
    public:
        static T& getInstance() {
            std::call_once(initFlag_, []() {
                instance_.reset(new T());
            });
            return *instance_;
        }

        virtual void initialize() {}
        virtual void shutdown() {}

    protected:
        Singleton() = default;
        virtual ~Singleton() = default;

    private:
        static std::unique_ptr<T> instance_;
        static std::once_flag initFlag_;
    };

    // 静态成员初始化
    template<typename T>
    std::unique_ptr<T> Singleton<T>::instance_;

    template<typename T>
    std::once_flag Singleton<T>::initFlag_;

    // RAII 守卫
    template<typename T>
    class SingletonGuard {
    public:
        SingletonGuard() {
            T::getInstance().initialize();
        }
        
        ~SingletonGuard() {
            T::getInstance().shutdown();
        }
        
        SingletonGuard(const SingletonGuard&) = delete;
        SingletonGuard& operator=(const SingletonGuard&) = delete;
    };
}