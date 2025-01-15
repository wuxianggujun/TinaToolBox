#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>
#include <chrono>

namespace TinaToolBox {

template <typename T>
class ThreadSafeQueue {
public:
    // 使用 priority_queue 作为内部容器
    using PriorityQueue = std::priority_queue<T, std::vector<T>, std::less<T>>;

    bool empty() const {
        std::lock_guard<std::mutex> lock{_queue_mutex};
        return _queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock{_queue_mutex};
        return _queue.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock{_queue_mutex};
        _queue = PriorityQueue();  // 创建新的空优先队列
        _queue_cv.notify_all();  // 通知所有等待的线程
    }

    void push(const T& value) {
        {
            std::lock_guard<std::mutex> lock{_queue_mutex};
            _queue.push(value);
        }
        _queue_cv.notify_one();  // 在锁外通知
    }

    void push(T&& value) {
        {
            std::lock_guard<std::mutex> lock{_queue_mutex};
            _queue.push(std::move(value));
        }
        _queue_cv.notify_one();  // 在锁外通知
    }

    // 批量弹出
    size_t try_pop_batch(std::vector<T>& out, size_t max_items) {
        std::lock_guard<std::mutex> lock{_queue_mutex};
        if (_queue.empty()) {
            return 0;
        }

        size_t items_to_pop = std::min(max_items, _queue.size());
        out.clear();
        out.reserve(items_to_pop);

        for (size_t i = 0; i < items_to_pop; ++i) {
            if (_queue.empty()) break;  // 安全检查
            out.push_back(std::move(const_cast<T&>(_queue.top())));
            _queue.pop();
        }

        return out.size();  // 返回实际弹出的数量
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock{_queue_mutex};
        if (_queue.empty()) {
            return false;
        }
        
        value = std::move(const_cast<T&>(_queue.top()));
        _queue.pop();
        return true;
    }

    T pop() {
        std::unique_lock<std::mutex> lock{_queue_mutex};
        while (_queue.empty()) {  // 使用while循环防止虚假唤醒
            _queue_cv.wait(lock);
        }
        
        T value = std::move(const_cast<T&>(_queue.top()));
        _queue.pop();
        return value;
    }

    // 添加带超时的pop
    template<typename Rep, typename Period>
    bool pop_for(T& value, const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock{_queue_mutex};
        if (!_queue_cv.wait_for(lock, timeout, [this] { return !_queue.empty(); })) {
            return false;
        }
        
        value = std::move(const_cast<T&>(_queue.top()));
        _queue.pop();
        return true;
    }

private:
    mutable std::mutex _queue_mutex;
    std::condition_variable _queue_cv;
    PriorityQueue _queue;
};

} // namespace TinaToolBox

