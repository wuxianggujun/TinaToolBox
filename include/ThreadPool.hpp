#pragma once
#include <atomic>
#include <functional>
#include <future>
#include <thread>
#include <vector>
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

            Task() : priority(TaskPriority::Normal) {}

            explicit Task(std::function<void()> f, TaskPriority p = TaskPriority::Normal)
                : func(std::move(f)), priority(p) {}

            bool operator>(const Task& other) const {
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

    public:
        explicit ThreadPool(size_t threads = std::thread::hardware_concurrency()) {
            workers_.reserve(threads);
            for (size_t i = 0; i < threads; ++i) {
                workers_.emplace_back(&ThreadPool::workerThread, this);
            }
        }

        ~ThreadPool() {
            is_active_ = false;
            task_queue_.clear();
            for (auto& worker : workers_) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
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
        }

        void waitForAll() {
            waiting_for_all_ = true;
            while (true) {
                if (task_queue_.empty() && active_tasks_ == 0) {
                    break;
                }
                std::this_thread::yield();
            }
            waiting_for_all_ = false;
        }

        const PoolStats& getStats() const { return stats_; }

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