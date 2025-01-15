#include <gtest/gtest.h>
#include <chrono>
#include <string>
#include <vector>
#include <future>
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
    for (int i = 0; i < 4; ++i) {
        futures.push_back(std::async(std::launch::async, [&] {
            for (int j = 0; j < NUM_TASKS; ++j) {
                pool.post([&counter] {
                    ++counter;
                });
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
