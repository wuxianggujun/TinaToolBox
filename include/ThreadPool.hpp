#pragma once
#include <atomic>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace TinaToolBox {
    struct use_future_tag {
    };

    template<class Fn>
    constexpr decltype(auto) use_future(Fn &&func) {
        return std::make_tuple(use_future_tag{}, std::forward<Fn>(func));
    }

    class ThreadPool {
    public:
        explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency()): is_active_(true) {
            workers_.reserve(num_threads);
            for (size_t i = 0; i < num_threads; ++i) {
                workers_.emplace_back(&ThreadPool::workerThread, this);
            }
        }

   
        ~ThreadPool() {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                is_active_ = false;
            }
            condition_variable_.notify_all();
            for (auto& worker : workers_) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }

        ThreadPool(const ThreadPool &) = delete;

        ThreadPool &operator=(const ThreadPool &) = delete;

        // 提交任务到线程池(无返回值版本)
        template<class Fn>
        void post(Fn &&func) {
            static_assert(std::is_invocable_v<std::decay_t<Fn> >,
                          "Function must be invocable without arguments");

            std::packaged_task<void()> task(std::forward<Fn>(func)); {
                std::unique_lock<std::mutex> lock(mutex_);
                pending_tasks_.emplace_back(std::move(task));
            }
            condition_variable_.notify_one();
        }


        // 提交任务到线程池(有返回值版本)
        template<class Fn>
        auto post(std::tuple<use_future_tag, Fn> &&t) {
            auto &&[_, func] = std::move(t);
            using return_type = std::invoke_result_t<std::decay_t<Fn> >;

            auto task = std::make_shared<std::packaged_task<return_type()> >(
                std::forward<Fn>(func)
            );
            auto future = task->get_future();

            post([task]() { (*task)(); });
            return future;
        }

        // 等待所有任务完成
        void waitForAll() {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_variable_.wait(lock, [this] {
                return pending_tasks_.empty() && (active_tasks_ == 0);
            });
        }

        // 获取工作线程数量
        size_t threadCount() const noexcept {
            return workers_.size();
        }

        // 获取当前等待执行的任务数量
        size_t pendingTaskCount() const noexcept {
            std::unique_lock<std::mutex> lock(mutex_);
            return pending_tasks_.size();
        }

        // 获取当前活跃的任务数量
        size_t activeTaskCount() const noexcept {
            return active_tasks_.load();
        }

    private:
        void workerThread() {
            while (true) {
                std::packaged_task<void()> task; {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_variable_.wait(lock, [this] {
                        return !is_active_ || !pending_tasks_.empty();
                    });

                    if (!is_active_ && pending_tasks_.empty()) {
                        return;
                    }

                    task = std::move(pending_tasks_.front());
                    pending_tasks_.pop_front();
                }

                ++active_tasks_;
                task();
                --active_tasks_;

                condition_variable_.notify_all(); // 通知waitForAll
            }
        }

        std::atomic<bool> is_active_;
        std::vector<std::thread> workers_;
        mutable std::mutex mutex_;
        std::condition_variable condition_variable_;
        std::deque<std::packaged_task<void()> > pending_tasks_;
        std::atomic<size_t> active_tasks_{0};
    };

    // 辅助函数：提交任务到线程池
    template<class Executor, class Fn>
    void post(Executor &exec, Fn &&func) {
        exec.post(std::forward<Fn>(func));
    }

    template<class Executor, class Fn>
    [[nodiscard]] decltype(auto) post(Executor &exec, std::tuple<use_future_tag, Fn> &&tpl) {
        return exec.post(std::move(tpl));
    }
}
