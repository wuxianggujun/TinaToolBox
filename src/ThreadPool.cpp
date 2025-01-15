#include "ThreadPool.hpp"

namespace TinaToolBox {
    void ThreadPool::workerThread() {
        while (is_active_) {
            std::vector<Task> tasks;
            Task task;

            // 尝试获取任务
            if (task_queue_.try_pop(task)) {
                ++active_tasks_;
                
                auto start = std::chrono::steady_clock::now();
                try {
                    task.func();
                    ++stats_.tasks_completed;
                } catch (...) {
                    ++stats_.tasks_failed;
                }
                auto end = std::chrono::steady_clock::now();
                stats_.total_task_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                
                --active_tasks_;
                
                // 如果正在等待所有任务完成，检查是否所有任务都已完成
                if (waiting_for_all_ && active_tasks_ == 0 && task_queue_.empty()) {
                    continue;
                }
            } else {
                // 如果没有任务，短暂休眠避免CPU空转
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
}