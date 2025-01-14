#pragma once
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

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
            std::packaged_task<void()> task;
            TaskPriority priority;

            Task() : priority(TaskPriority::Normal) {}

            explicit Task(std::packaged_task<void()> t, TaskPriority p = TaskPriority::Normal)
                : task(std::move(t)), priority(p) {}

            bool operator>(const Task& other) const {
                return priority > other.priority;
            }
        };

        static constexpr size_t TASK_BATCH_SIZE = 32;
        
        std::atomic_bool is_active_{true};
        std::vector<std::thread> workers_;
        mutable std::mutex mutex_;
        std::condition_variable condition_;
        std::vector<std::deque<Task>> local_queues_;
        std::atomic_size_t active_tasks_{0};
        PoolStats stats_{};

    public:
        explicit ThreadPool(size_t threads = std::thread::hardware_concurrency()) 
            : local_queues_(threads) {
            workers_.reserve(threads);
            for (size_t i = 0; i < threads; ++i) {
                workers_.emplace_back(&ThreadPool::workerThread, this, i);
            }
        }

        ~ThreadPool() {
            {
                std::lock_guard<std::mutex> lock(mutex_);
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
            static std::atomic<size_t> index{0};
            auto worker_id = index++ % workers_.size();
            
            Task task(std::packaged_task<void()>(std::forward<F>(f)), priority);
            {
                std::lock_guard<std::mutex> lock(mutex_);
                local_queues_[worker_id].push_back(std::move(task));
            }
            condition_.notify_one();
        }

        template<typename Iterator>
        void batch_post(Iterator begin, Iterator end, TaskPriority priority = TaskPriority::Normal) {
            const size_t total_tasks = std::distance(begin, end);
            const size_t tasks_per_thread = (total_tasks + workers_.size() - 1) / workers_.size();
            
            auto it = begin;
            for (size_t i = 0; i < workers_.size() && it != end; ++i) {
                size_t count = 0;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    while (count < tasks_per_thread && it != end) {
                        local_queues_[i].emplace_back(std::packaged_task<void()>(*it), priority);
                        ++it;
                        ++count;
                    }
                }
                condition_.notify_one();
            }
        }

        void waitForAll() {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] {
                // 检查所有本地队列是否为空
                for (const auto& queue : local_queues_) {
                    if (!queue.empty()) return false;
                }
                return active_tasks_ == 0;
            });
        }

    private:
        void workerThread(size_t id);
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
