#pragma once
#include <atomic>
#include <functional>
#include <future>
#include <thread>
#include <vector>
#include <queue>
#include "ThreadSafeQueue.hpp"

namespace TinaToolBox {
    struct use_future_tag {
    };

    template<class Fn>
    constexpr decltype(auto) use_future(Fn &&func) {
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
            std::exception_ptr exception;

            Task() : priority(TaskPriority::Normal) {
            }

            explicit Task(std::function<void()> f, TaskPriority p = TaskPriority::Normal)
                : func(std::move(f)), priority(p) {
            }

            bool operator>(const Task &other) const {
                return priority < other.priority;
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

    public:
        explicit ThreadPool(size_t threads = std::thread::hardware_concurrency()) {
            workers_.reserve(threads);
            for (size_t i = 0; i < threads; ++i) {
                workers_.emplace_back(&ThreadPool::workerThread, this);
            }
        }

        ~ThreadPool() {
            shutdown();
        }

        ThreadPool(const ThreadPool &) = delete;

        ThreadPool &operator=(const ThreadPool &) = delete;

        size_t threadCount() const { return workers_.size(); }

        template<typename F>
        auto post(use_future_tag, F &&f) -> std::future<std::invoke_result_t<F> > {
            using return_type = std::invoke_result_t<F>;
            auto task = std::make_shared<std::packaged_task<return_type()> >(
                std::forward<F>(f)
            );
            std::future<return_type> future = task->get_future();

            post([task]() { (*task)(); });
            return future;
        }

        template<class F>
        void post(F &&f, TaskPriority priority = TaskPriority::Normal) {
            task_queue_.push(Task(std::forward<F>(f), priority));
        }

        template<typename F>
        auto submit(F&& f, TaskPriority priority = TaskPriority::Normal)
            -> std::future<std::invoke_result_t<F>>
        {
            using return_type = std::invoke_result_t<F>;
            
            // 特化处理void返回类型
            if constexpr (std::is_void_v<return_type>) {
                auto task = std::make_shared<std::packaged_task<void()>>(
                    [f = std::forward<F>(f), this]() {
                        try {
                            f();
                            ++stats_.tasks_completed;
                        } catch (...) {
                            ++stats_.tasks_failed;
                            throw;
                        }
                    }
                );
                std::future<void> future = task->get_future();
                post([task]() { (*task)(); }, priority);
                return future;
            } else {
                auto task = std::make_shared<std::packaged_task<return_type()>>(
                    [f = std::forward<F>(f), this]() -> return_type {
                        try {
                            auto result = f();
                            ++stats_.tasks_completed;
                            return result;
                        } catch (...) {
                            ++stats_.tasks_failed;
                            throw;
                        }
                    }
                );
                std::future<return_type> future = task->get_future();
                post([task]() { (*task)(); }, priority);
                return future;
            }
        }
        
        template<typename Iterator>
        void batch_post(Iterator begin, Iterator end, TaskPriority priority = TaskPriority::Normal) {
            for (auto it = begin; it != end; ++it) {
                task_queue_.push(Task(*it, priority));
            }
            cv_all_tasks_done_.notify_all();
        }
        
        void waitForAll() {
            std::unique_lock<std::mutex> lock(all_tasks_done_mutex_);
            waiting_for_all_ = true;
            while (!task_queue_.empty() || active_tasks_ > 0) {
                cv_all_tasks_done_.wait(lock);
            }
            waiting_for_all_ = false;
        }

        const PoolStats &getStats() const { return stats_; }

        void shutdown() {
            is_active_ = false;
            task_queue_.clear();
            cv_all_tasks_done_.notify_all();

            for (auto &worker: workers_) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }

    private:
        void workerThread();
    };

    template<typename F>
    auto post(ThreadPool &pool, std::tuple<use_future_tag, F> &&task)
        -> std::future<std::invoke_result_t<std::decay_t<F> > > {
        return pool.post(use_future_tag{}, std::forward<F>(std::get<1>(task)));
    }
} // namespace TinaToolBox
