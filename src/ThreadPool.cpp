#include "ThreadPool.hpp"

namespace TinaToolBox {
    void ThreadPool::workerThread() {
        while (is_active_) {
            Task task;
            bool got_task = false;
            
            {
                std::unique_lock<std::mutex> lock(all_tasks_done_mutex_);
                if (!task_queue_.empty()) {
                    got_task = task_queue_.try_pop(task);
                    if (got_task) {
                        ++active_tasks_;
                    }
                } else if (is_active_) {
                    cv_all_tasks_done_.wait_for(lock, std::chrono::microseconds(100));
                    continue;
                }
            }

            if (got_task) {
                auto start = std::chrono::steady_clock::now();
                try {
                    task.func();
                } catch (...) {
                    task.exception = std::current_exception();
                }
                auto end = std::chrono::steady_clock::now();
                stats_.total_task_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                
                {
                    std::unique_lock<std::mutex> lock(all_tasks_done_mutex_);
                    --active_tasks_;
                    if (waiting_for_all_ && active_tasks_ == 0 && task_queue_.empty()) {
                        cv_all_tasks_done_.notify_all();
                    }
                }
            }
        }
    }
}