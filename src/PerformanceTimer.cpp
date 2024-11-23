//
// Created by wuxianggujun on 2024/11/23.
//

#include "PerformanceTimer.hpp"


PerformanceTimer::PerformanceTimer(const std::string &name): name_(name) {
}

PerformanceTimer::~PerformanceTimer() = default;

void PerformanceTimer::start() {
    start_time_ = std::chrono::high_resolution_clock::now();
}

double PerformanceTimer::stop() {
    end_time_ = std::chrono::high_resolution_clock::now();
    if (!name_.empty()) {
        log_performance();
    }
    return duration();
}

double PerformanceTimer::duration() const {
    auto duration = std::chrono::duration_cast<std::chrono::duration<double> >(
        end_time_ - start_time_
    );
    return duration.count(); // 直接返回秒数
}

ScopeTimer::ScopeTimer(const std::string &name): timer_(name) {
    timer_.start();
}

ScopeTimer::~ScopeTimer() {
    timer_.stop();
}

void PerformanceTimer::log_performance() const {
    spdlog::info("{} - Execution time:{:.4f} seconds", name_, duration());
}
