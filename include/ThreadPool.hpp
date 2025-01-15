#pragma once
#include <atomic>
#include <functional>
#include <future>
#include <thread>
#include <vector>
#include <queue> 
#include "ThreadSafeQueue.hpp"

namespace TinaToolBox {
    struct use_future_tag {};

    template<class Fn>
    constexpr decltype(auto) use_future(Fn&& func) {
        return std::make_tuple(use_future_tag{}, std::forward<Fn>(func));
    }

    class ThreadPool {
    public:
        enum class TaskPriority {
            Low,
            Normal,
            High
        };

        struct PoolStats {
            std::atomic_size_t tasks_completed{0};
            std::atomic_size_t tasks_failed{0};
            std::atomic_uint64_t total_task_time{0};
        };

    private:
        struct Task {
            std::function<void()> func;
            TaskPriority priority;
            std::exception_ptr exception; // 用于存储异常

            Task() : priority(TaskPriority::Normal) {}

            explicit Task(std::function<void()> f, TaskPriority p = TaskPriority::Normal)
                : func(std::move(f)), priority(p) {}

            bool operator<(const Task& other) const {
                return priority > other.priority;
            }
        };

        std::atomic_bool is_active_{true};
        std::atomic_bool waiting_for_all_{false};
        std::vector<std::thread> workers_;
        ThreadSafeQueue<Task> task_queue_;
        std::atomic_size_t active_tasks_{0};
        PoolStats stats_{};
        static constexpr size_t BATCH_SIZE = 100;
        std::condition_variable cv_all_tasks_done_;
        std::mutex all_tasks_done_mutex_;
        std::condition_variable cv_new_task_;  // 用于通知新任务的条件变量
        std::mutex task_queue_mutex_;          // 与 task_queue_ 相关的互斥锁

    public:
        explicit ThreadPool(size_t threads = std::thread::hardware_concurrency()) : task_queue_mutex_() {
            workers_.reserve(threads);
            for (size_t i = 0; i < threads; ++i) {
                workers_.emplace_back(&ThreadPool::workerThread, this);
            }
        }

        ~ThreadPool() {
           shutdown();
        }

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        size_t threadCount() const { return workers_.size(); }

        // 添加post函数的重载，处理带future_tag的任务
        template<typename F>
        auto post(use_future_tag, F&& f) -> std::future<std::invoke_result_t<F>> {
            using return_type = std::invoke_result_t<F>;
            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::forward<F>(f)
            );
            std::future<return_type> future = task->get_future();

            post([task]() { (*task)(); });
            return future;
        }

        template<class F>
        void post(F&& f, TaskPriority priority = TaskPriority::Normal) {
            task_queue_.push(Task(std::forward<F>(f), priority));
            cv_new_task_.notify_all(); // 通知所有等待的线程
        }

        template<typename F>
        auto submit(F&& f, TaskPriority priority = TaskPriority::Normal)
            -> std::future<std::invoke_result_t<F>>
        {
            using return_type = std::invoke_result_t<F>;
            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::forward<F>(f)
            );
            std::future<return_type> future = task->get_future();

            post([task]() { (*task)(); }, priority);
            return future;
        }

        template<typename Iterator>
        void batch_post(Iterator begin, Iterator end, TaskPriority priority = TaskPriority::Normal) {
            for (auto it = begin; it != end; ++it) {
                task_queue_.push(Task(*it, priority));
            }
            cv_new_task_.notify_all(); // 通知所有等待的线程
        }

        void waitForAll() {
            std::unique_lock<std::mutex> lock(all_tasks_done_mutex_);
            cv_all_tasks_done_.wait(lock, [this] { return task_queue_.empty() && active_tasks_ == 0; });
        }

        const PoolStats& getStats() const { return stats_; }

        void shutdown() {
            is_active_ = false; // 设置线程池不再活跃
            cv_new_task_.notify_all(); // 通知所有等待的线程
            task_queue_.clear(); // 清空任务队列
            cv_all_tasks_done_.notify_all(); // 通知等待中的线程

            for (auto& worker : workers_) {
                if (worker.joinable()) {
                    worker.join(); // 等待所有线程结束
                }
            }
        }

    private:
        void workerThread();
    };

    // 添加全局post函数
    template<typename F>
    auto post(ThreadPool& pool, std::tuple<use_future_tag, F>&& task)
        -> std::future<std::invoke_result_t<std::decay_t<F>>>
    {
        return pool.post(use_future_tag{}, std::forward<F>(std::get<1>(task)));
    }

} // namespace TinaToolBox