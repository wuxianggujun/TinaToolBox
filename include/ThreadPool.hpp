#pragma once
#include <atomic>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/***
 *
*支持异步任务提交
* 任务优先级调度
* 工作窃取负载均衡
* 性能监控和统计
* 线程池状态管理（启动/关闭）
* 支持批量任务处理
*
 */
namespace TinaToolBox {
    struct use_future_tag {
    };

    template<class Fn>
    constexpr decltype(auto) use_future(Fn &&func) {
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
            std::atomic_size_t tasks_stolen{0};

            // 添加默认构造函数
            PoolStats() = default;

            // 添加拷贝构造函数
            PoolStats(const PoolStats &other)
                : tasks_completed(other.tasks_completed.load())
                  , tasks_failed(other.tasks_failed.load())
                  , total_task_time(other.total_task_time.load())
                  , tasks_stolen(other.tasks_stolen.load()) {
            }

            void reset() {
                tasks_completed = 0;
                tasks_failed = 0;
                total_task_time = 0;
                tasks_stolen = 0;
            }
        };

    private:
        struct Task;

        // 工作线程本地数据
        struct WorkerData {
            std::deque<Task> local_queue; // 每个线程的本地任务队列
            std::mutex local_mutex; // 本地队列的互斥锁
            std::condition_variable local_condition_variable;
            size_t worker_id{}; // 工作线程ID
            std::atomic_size_t tasks_processed{0}; // 已处理任务计数
        };

        struct Task {
            std::packaged_task<void()> task;
            TaskPriority priority;
            std::chrono::steady_clock::time_point created_time;

            // 添加默认构造函数
            Task() : priority(TaskPriority::Normal), created_time(std::chrono::steady_clock::now()) {
            }

            explicit Task(std::packaged_task<void()> t, TaskPriority p = TaskPriority::Normal): task(std::move(t)),
                priority(p),
                created_time(std::chrono::steady_clock::now()) {
            }

            bool operator>(const Task &other) const {
                if (priority != other.priority) {
                    return priority > other.priority;
                }
                // 同优先级FIFO
                return created_time < other.created_time;
            }
        };

        // 使用透明比较器
        using TaskQueue = std::priority_queue<Task, std::vector<Task>, std::greater<> >;

    public:
        explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency()): is_active_(true), workers_()
            , worker_data_(num_threads) {
            // 创建工作线程
            workers_.reserve(num_threads);
            // 初始化每个工作线程的本地数据
            for (size_t i = 0; i < num_threads; ++i) {
                // 初始化工作线程数据
                worker_data_[i] = std::make_unique<WorkerData>();
                worker_data_[i]->worker_id = i;
                // 创建对应的工作线程
                workers_.emplace_back(&ThreadPool::workerThread, this, i);
            }
        }


        ~ThreadPool() { {
                std::unique_lock<std::mutex> lock(mutex_);
                is_active_ = false;
            }
            condition_variable_.notify_all();
            for (auto &worker: workers_) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }

        ThreadPool(const ThreadPool &) = delete;

        ThreadPool &operator=(const ThreadPool &) = delete;

        // 提交任务到线程池(无返回值版本)
        template<class Fn>
        void post(Fn &&func, TaskPriority priority = TaskPriority::Normal) {
            static_assert(std::is_invocable_v<std::decay_t<Fn> >,
                          "Function must be invocable without arguments");

            std::packaged_task<void()> packagedTask(std::forward<Fn>(func)); {
                std::unique_lock<std::mutex> lock(mutex_);
                pending_tasks_.emplace(std::move(packagedTask), priority);
            }
            condition_variable_.notify_one();
        }


        // 提交任务到线程池(有返回值版本)
        template<class Fn>
        decltype(auto) post(std::tuple<use_future_tag, Fn> &&t, TaskPriority priority = TaskPriority::Normal) {
            auto &&[_, func] = std::move(t);
            using return_type = std::invoke_result_t<std::decay_t<Fn> >;

            auto task = std::make_shared<std::packaged_task<return_type()> >(
                std::forward<Fn>(func)
            );
            auto future = task->get_future();

            post([task]() { (*task)(); }, priority);
            return future;
        }

        // 批量提交任务
        template<typename Iterator>
        void batch_post(Iterator begin, Iterator end, TaskPriority priority = TaskPriority::Normal) {
            std::unique_lock<std::mutex> lock(mutex_);
            for (auto it = begin; it != end; ++it) {
                pending_tasks_.emplace(std::packaged_task<void()>(*it), priority);
            }
            lock.unlock();
            condition_variable_.notify_all();
        }

        // 等待所有任务完成
        void waitForAll() {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_variable_.wait(lock, [this] {
                return pending_tasks_.empty() && (active_tasks_ == 0);
            });
        }

        // 获取线程池统计信息
        PoolStats getStats() const {
            return stats_;
        }

        // 重置统计信息
        void resetStats() {
            stats_.reset();
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
        // 工作窃取
        Task stealTask(size_t worker_id) {
            for (size_t i = 0; i < workers_.size(); ++i) {
                if (i == worker_id) continue;

                std::lock_guard<std::mutex> lock(worker_data_[i]->local_mutex);
                if (!worker_data_[i]->local_queue.empty()) {
                    Task task = std::move(worker_data_[i]->local_queue.front());
                    worker_data_[i]->local_queue.pop_front();
                    ++stats_.tasks_stolen;
                    return task;
                }
            }
            return Task(std::packaged_task<void()>{}, TaskPriority::Normal);
        }

        void workerThread(size_t worker_id) {
            auto &local_data = *worker_data_[worker_id];

            while (true) {
                Task task; {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_variable_.wait(lock, [this] {
                        return !is_active_ || !pending_tasks_.empty();
                    });

                    if (!is_active_ && pending_tasks_.empty()) {
                        return;
                    }

                    // 优先处理全局队列中的高优先级任务
                    if (!pending_tasks_.empty()) {
                        task = std::move(const_cast<Task &>(pending_tasks_.top()));
                        pending_tasks_.pop();
                    } else {
                        // 尝试从其他线程窃取任务
                        task = stealTask(worker_id);
                        if (!task.task.valid()) {
                            continue;
                        }
                    }
                }

                // 执行任务并记录统计信息
                ++active_tasks_;
                auto start_time = std::chrono::steady_clock::now();

                try {
                    task.task();
                    ++stats_.tasks_completed;
                    ++local_data.tasks_processed;
                } catch (...) {
                    ++stats_.tasks_failed;
                }

                auto end_time = std::chrono::steady_clock::now();
                stats_.total_task_time += std::chrono::duration_cast<std::chrono::nanoseconds>(
                    end_time - start_time).count();

                --active_tasks_;
                condition_variable_.notify_all();
            }
        }

        std::atomic_bool is_active_;
        std::vector<std::thread> workers_;
        mutable std::mutex mutex_;
        std::condition_variable condition_variable_;
        TaskQueue pending_tasks_;
        std::atomic_size_t active_tasks_{0};
        std::vector<std::unique_ptr<WorkerData> > worker_data_;
        PoolStats stats_;
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
