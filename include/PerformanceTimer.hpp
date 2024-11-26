//
// Created by wuxianggujun on 2024/11/23.
//

#ifndef TINA_TOOL_BOX_PERFORMANCE_TIMER_HPP
#define TINA_TOOL_BOX_PERFORMANCE_TIMER_HPP

#include <chrono>
#include <functional>
#include <string>
#include <memory>
#include <spdlog/spdlog.h>

class ScopeTimer;

class PerformanceTimer {
public:
    explicit PerformanceTimer(const std::string &name = "");

    ~PerformanceTimer();

    void start();

    double stop();

    double duration() const;
    
    template<class F>
    static decltype(auto) timer(const std::string &name, F &&func) {
        return [name,func = std::forward<F>(func)]<typename... T0>(T0 &&... args) -> decltype(auto) {
            PerformanceTimer timer(name);
            timer.start();
            auto result = func(std::forward<T0>(args)...);
            timer.stop();
            return result;
        };
    }

private:
    std::string name_;
    std::chrono::steady_clock::time_point start_time_{};
    std::chrono::steady_clock::time_point end_time_{};

    void log_performance() const;
};

class ScopeTimer {
public:
    explicit ScopeTimer(const std::string &name);

    ~ScopeTimer();

private:
    PerformanceTimer timer_;
};

#endif //TINA_TOOL_BOX_PERFORMANCE_TIMER_HPP
