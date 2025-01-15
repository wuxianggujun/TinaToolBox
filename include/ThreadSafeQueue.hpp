#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <optional>

namespace TinaToolBox {
    template<class T>
    class ThreadSafeQueue {
    public:
        bool empty() const {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            return queue_.empty();
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            return queue_.size();
        }

        void clear() {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            queue_ = {};
        }

        bool try_pop(T &value) {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (queue_.empty()) {
                return false;
            }
            value = std::move(queue_.front());
            queue_.pop();
            return true;
        }

        std::optional<T> try_pop() {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (queue_.empty()) {
                return std::nullopt;
            }
            T value = std::move(queue_.front());
            queue_.pop();
            return value;
        }

        template<typename Rep, typename Period>
        std::optional<T> pop_for(const std::chrono::duration<Rep, Period> &timeout) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (!condition_variable_.wait_for(lock, timeout, [this] {
                return !queue_.empty();
            })) {
                return std::nullopt;
            }

            T value = std::move(queue_.front());
            queue_.pop();
            return value;
        }

        void push(T const &value) {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            queue_.push(value);
            condition_variable_.notify_one();
        }

        void push(T &&value) {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            queue_.push(std::move(value));
            condition_variable_.notify_one();
        }

        T pop() {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_variable_.wait(lock, [this] {
                return !queue_.empty();
            });
            T value = std::move(queue_.front());
            queue_.pop();
            return value;
        }

    private:
        mutable std::mutex queue_mutex_;
        std::queue<T> queue_;
        std::condition_variable condition_variable_;
    };
}
