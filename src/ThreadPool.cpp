#include "ThreadPool.hpp"

namespace TinaToolBox {
    void ThreadPool::workerThread(size_t id) {
        while (true) {
            Task task;
            bool has_task = false;
            
            // 首先检查本地队列
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (!local_queues_[id].empty()) {
                    task = std::move(local_queues_[id].front());
                    local_queues_[id].pop_front();
                    has_task = true;
                }
            }
            
            // 如果本地队列为空，尝试从其他线程窃取任务
            if (!has_task) {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this, id, &task, &has_task] {
                    if (!is_active_) return true;
                    
                    // 检查所有队列
                    for (size_t i = 0; i < local_queues_.size(); ++i) {
                        if (!local_queues_[i].empty()) {
                            task = std::move(local_queues_[i].front());
                            local_queues_[i].pop_front();
                            has_task = true;
                            return true;
                        }
                    }
                    return false;
                });
                
                if (!is_active_ && !has_task) {
                    return;
                }
            }

            if (has_task) {
                ++active_tasks_;
                auto start = std::chrono::steady_clock::now();

                try {
                    task.task();
                    ++stats_.tasks_completed;
                } catch (...) {
                    ++stats_.tasks_failed;
                }

                auto end = std::chrono::steady_clock::now();
                stats_.total_task_time += std::chrono::duration_cast<std::chrono::nanoseconds>(
                    end - start).count();

                --active_tasks_;
                condition_.notify_all();
            }
        }
    }
}
