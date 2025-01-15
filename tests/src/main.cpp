#include <gtest/gtest.h>
#include <chrono>
#include <string>
#include <vector>
#include <future>
#include <random>
#include "ThreadPool.hpp"

using namespace TinaToolBox;
using namespace std::chrono_literals;

class ThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(ThreadPoolTest, ConstructorAndDestructor) {
    // 基本构造
    ThreadPool pool(4);
    EXPECT_EQ(pool.threadCount(), 4);

    // 默认构造
    ThreadPool defaultPool;
    EXPECT_EQ(defaultPool.threadCount(), std::thread::hardware_concurrency());
}

TEST_F(ThreadPoolTest, PostTaskWithoutResult) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};

    for (int i = 0; i < 100; ++i) {
        pool.post([&counter] {
            ++counter;
        });
    }

    pool.waitForAll();
    EXPECT_EQ(counter.load(), 100);
}

TEST_F(ThreadPoolTest, PostTaskWithResult) {
    ThreadPool pool(2);

    auto future = post(pool, use_future([] {
        return 42;
    }));

    EXPECT_EQ(future.get(), 42);
}

TEST_F(ThreadPoolTest, MultipleTasksWithResults) {
    ThreadPool pool(4);
    std::vector<std::future<int> > futures;
    const int NUM_TASKS = 100;

    for (int i = 0; i < NUM_TASKS; ++i) {
        auto future = post(pool, use_future([i] {
            std::this_thread::sleep_for(1ms);
            return i;
        }));
        futures.push_back(std::move(future));
    }

    int sum = 0;
    for (auto &future: futures) {
        sum += future.get();
    }

    EXPECT_EQ(sum, (NUM_TASKS - 1) * NUM_TASKS / 2); // 等差数列求和
}

TEST_F(ThreadPoolTest, ExceptionHandling) {
    ThreadPool pool(2);

    auto future = post(pool, use_future([]() -> int {
        throw std::runtime_error("Test exception");
        return 0;
    }));

    EXPECT_THROW(future.get(), std::runtime_error);
}

TEST_F(ThreadPoolTest, StressTest) {
    ThreadPool pool(std::thread::hardware_concurrency());  // 使用所有可用核心
    const int NUM_TASKS = 10000;
    std::atomic<int> counter{0};

    auto start = std::chrono::high_resolution_clock::now();

    // 使用批量提交而不是单个提交
    std::vector<std::function<void()>> tasks;
    tasks.reserve(NUM_TASKS);
    for (int i = 0; i < NUM_TASKS; ++i) {
        tasks.emplace_back([&counter] {
            ++counter;
        });
    }

    pool.batch_post(tasks.begin(), tasks.end());
    pool.waitForAll();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_EQ(counter.load(), NUM_TASKS);
    std::cout << "Processed " << NUM_TASKS << " tasks in " << duration.count() << "ms" << std::endl;
}

TEST_F(ThreadPoolTest, ConcurrentAccess) {
    ThreadPool pool(8);
    std::vector<std::future<void> > futures;
    std::atomic<int> counter{0};
    constexpr int NUM_TASKS = 1000;

    // 同时提交多个任务
    futures.reserve(8);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 10); // 1到10毫秒的随机延迟

    for (int i = 0; i < 4; ++i) {
        futures.push_back(std::async(std::launch::async, [&] {
            for (int j = 0; j < NUM_TASKS; ++j) {
                pool.post([&counter] {
                    ++counter;
                });
                std::this_thread::sleep_for(std::chrono::milliseconds(distrib(gen))); // 增加随机延迟
            }
        }));
    }

    // 等待所有提交完成
    for (auto &f: futures) {
        f.wait();
    }

    pool.waitForAll();
    EXPECT_EQ(counter.load(), NUM_TASKS * 4);
}

TEST_F(ThreadPoolTest, TaskPriority) {
    ThreadPool pool(1); // 使用单线程更容易观察优先级的影响
    std::vector<int> results;
    std::mutex results_mutex;

    pool.post([&] {
        std::lock_guard<std::mutex> lock(results_mutex);
        results.push_back(1);
    }, ThreadPool::TaskPriority::Low);

    pool.post([&] {
        std::lock_guard<std::mutex> lock(results_mutex);
        results.push_back(2);
    }, ThreadPool::TaskPriority::Normal);

    pool.post([&] {
        std::lock_guard<std::mutex> lock(results_mutex);
        results.push_back(3);
    }, ThreadPool::TaskPriority::High);

    pool.waitForAll();

    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], 3); // 高优先级的任务先执行
    EXPECT_EQ(results[1], 2); // 普通优先级的任务其次
    EXPECT_EQ(results[2], 1); // 低优先级的任务最后
}

// 测试 ThreadSafeQueue 的 try_pop_batch 的边界情况
TEST_F(ThreadPoolTest, ThreadSafeQueueTryPopBatchBoundary) {
    ThreadSafeQueue<int> queue;

    // 空队列
    std::vector<int> batch;
    EXPECT_EQ(queue.try_pop_batch(batch, 10), 0);
    EXPECT_TRUE(batch.empty());

    // 少量元素
    queue.push(1);
    queue.push(2);
    EXPECT_EQ(queue.try_pop_batch(batch, 10), 2);
    ASSERT_EQ(batch.size(), 2);
    EXPECT_EQ(batch[0], 1);
    EXPECT_EQ(batch[1], 2);
    batch.clear();

    // 多于 BATCH_SIZE 的元素
    for (int i = 0; i < 20; ++i) {
        queue.push(i);
    }
    EXPECT_EQ(queue.try_pop_batch(batch, 10), 10);
    ASSERT_EQ(batch.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(batch[i], i);
    }
}

TEST_F(ThreadPoolTest, Shutdown) {
    ThreadPool pool;
    std::atomic<int> counter{0};

    // 提交一些任务
    for (int i = 0; i < 10; ++i) {
        pool.post([&counter] {
            std::this_thread::sleep_for(10ms);
            ++counter;
        });
    }

    // 调用 shutdown
    pool.shutdown();

    // 尝试提交更多任务（应该被拒绝，但这里我们简单地忽略返回值）
    pool.post([&counter] {
        ++counter;
    });

    // 等待一段时间，确保之前的任务有机会执行
    std::this_thread::sleep_for(100ms);

    // 此时 counter 的值应该小于或等于 10，因为 shutdown 后的任务不会被执行
    EXPECT_LE(counter.load(), 10);
}

TEST_F(ThreadPoolTest, PoolStats) {
    ThreadPool pool;
    
    auto successTask = pool.submit([] {
        return 10;
    });
    
    EXPECT_EQ(successTask.get(), 10);

    auto failedTask = pool.submit([] {
        throw std::runtime_error("Failed task");
    });

    EXPECT_THROW(failedTask.get(), std::runtime_error);
    
    pool.waitForAll();
    
    // 使用常量引用接收返回值
    const auto& stats = pool.getStats(); 
    EXPECT_EQ(stats.tasks_completed, 1);
    EXPECT_EQ(stats.tasks_failed, 1);
    EXPECT_GT(stats.total_task_time, 0);
}