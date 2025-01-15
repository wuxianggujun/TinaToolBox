#include "ThreadPool.hpp"

namespace TinaToolBox {
    void ThreadPool::workerThread() {
        while (true) {
            std::vector<Task> tasks;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] { return !task_queue_.empty() || !is_active_; });

                if (!is_active_ && task_queue_.empty()) {
                    return;
                }

                // 批量取出任务
                for (size_t i = 0; i < BATCH_SIZE && !task_queue_.empty(); ++i) {
                    tasks.push_back(std::move(task_queue_.front()));
                    task_queue_.pop();
                }
                active_tasks_ += tasks.size(); // 只有在真正取出任务后才增加 active_tasks_
            }

            for (auto& task : tasks) {
                auto start = std::chrono::steady_clock::now();
                try {
                    task.func();
                    ++stats_.tasks_completed;
                } catch (...) {
                    ++stats_.tasks_failed;
                }
                auto end = std::chrono::steady_clock::now();
                stats_.total_task_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    --active_tasks_;
                    // 如果线程池正在等待所有任务完成，并且当前没有活动任务，则通知主线程
                    if (waiting_for_all_ && active_tasks_ == 0 && task_queue_.empty()) {
                        condition_.notify_all();
                    }
                }
            }
        }
    }
}