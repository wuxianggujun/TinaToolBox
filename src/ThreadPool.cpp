#include "ThreadPool.hpp"

namespace TinaToolBox {
    void ThreadPool::workerThread() {
        while (is_active_) {
            std::vector<Task> tasks;
            {
                std::unique_lock<std::mutex> lock(task_queue_mutex_);
                cv_new_task_.wait(lock, [this] { return !task_queue_.empty() || !is_active_; });

                if (!is_active_ && task_queue_.empty()) {
                    return;
                }

                size_t numTasks = task_queue_.try_pop_batch(tasks, BATCH_SIZE);
                active_tasks_ += numTasks;
            }

            for (auto& task : tasks) {
                auto start = std::chrono::steady_clock::now();
                try {
                    task.func();
                    auto end = std::chrono::steady_clock::now();
                    stats_.total_task_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                    ++stats_.tasks_completed;
                } catch (...) {
                    ++stats_.tasks_failed;
                    task.exception = std::current_exception();
                }
            }

            if (active_tasks_.fetch_sub(static_cast<int>(tasks.size())) == tasks.size() && task_queue_.empty()) {
                cv_all_tasks_done_.notify_all();
            }
        }
    }
}