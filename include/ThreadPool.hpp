#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <queue>
#include <thread>
#include <vector>
#include <mutex>

namespace TinaToolBox {
    struct use_future_tag {};

    template<class Fn>
    constexpr decltype(auto) use_future(Fn&& func) {
        return std::make_tuple(use_future_tag{}, std::forward<Fn>(func));
    }

    class ThreadPool {
    public:
        // 任务优先级枚举
        enum class TaskPriority {
            Low,
            Normal,
            High
        };

        // 线程池统计信息
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
        std::atomic_bool waiting_for_all_{false}; // 用于标记是否等待所有任务完成
        std::vector<std::thread> workers_;
        std::queue<Task> task_queue_;
        std::mutex queue_mutex_;
        std::condition_variable condition_;
        std::atomic_size_t active_tasks_{0};
        PoolStats stats_{};
        static constexpr size_t BATCH_SIZE = 100; // 每次处理的任务数量

    public:
        explicit ThreadPool(size_t threads = std::thread::hardware_concurrency()) {
            workers_.reserve(threads);
            for (size_t i = 0; i < threads; ++i) {
                workers_.emplace_back(&ThreadPool::workerThread, this);
            }
        }

        ~ThreadPool() {
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                is_active_ = false;
            }
            condition_.notify_all();
            for (auto& worker : workers_) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        size_t threadCount() const { return workers_.size(); }

        template<class F>
        void post(F&& f, TaskPriority priority = TaskPriority::Normal) {
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                task_queue_.push(Task(std::forward<F>(f), priority));
            }
            condition_.notify_one();
        }

        template<typename Iterator>
        void batch_post(Iterator begin, Iterator end, TaskPriority priority = TaskPriority::Normal) {
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                for (auto it = begin; it != end; ++it) {
                    task_queue_.push(Task(*it, priority));
                }
            }
            condition_.notify_all();
        }

        void waitForAll() {
            waiting_for_all_ = true; // 设置等待所有任务完成的标记
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this] { return task_queue_.empty() && active_tasks_ == 0; });
            waiting_for_all_ = false; // 重置标记
        }

    private:
        void workerThread();
    };

    template<class Fn>
    auto post(ThreadPool& pool, std::tuple<use_future_tag, Fn>&& t) {
        auto&& [_, func] = std::move(t);
        using return_type = std::invoke_result_t<std::decay_t<Fn>>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::forward<Fn>(func)
        );
        auto future = task->get_future();

        pool.post([task]() { (*task)(); });
        return future;
    }
} // namespace TinaToolBox