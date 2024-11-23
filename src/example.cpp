//
// Created by wuxianggujun on 2024/11/23.
//

#include <iostream>

#include "PerformanceTimer.hpp"
#include <thread>
#include <QSqlQuery>
#include "ExceptionHandler.hpp"
// 初始化日志
void init_logger() {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);
}

// 方式1：直接使用类
void test_direct() {
    PerformanceTimer timer("Direct Test");
    timer.start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    timer.stop();
}

// 方式2：使用RAII方式（推荐）
void test_scoped() {
    ScopeTimer timer("Scoped Test");
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// 方式3：使用装饰器方式
auto slow_function() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 42;
}


class DatabaseManager {
public:
    bool insertRecord(const QString& name, int age) {
        DBExceptionHandler dbHandler("插入记录失败");
        
        return dbHandler([&]() {
            QSqlQuery query;
            query.prepare("INSERT INTO users (name, age) VALUES (?, ?)");
            query.addBindValue(name);
            query.addBindValue(age);
            
            if (!query.exec()) {
                throw std::runtime_error(query.lastError().text().toStdString());
            }
            return true;
        });
    }

    QString findUserName(int id) {
        ExceptionHandler handler("查询用户失败");
        
        return handler([&]() {
            QSqlQuery query;
            query.prepare("SELECT name FROM users WHERE id = ?");
            query.addBindValue(id);
            
            if (!query.exec() || !query.next()) {
                throw std::invalid_argument("用户不存在");
            }
            return query.value(0).toString();
        });
    }
};

/*int main() {

    DatabaseManager db;
    db.insertRecord("hh",1);
    init_logger();

    // 测试直接使用
    test_direct();

    // 测试RAII方式
    test_scoped();

    // 测试装饰器方式
    auto timed_function = PerformanceTimer::timer("Decorator Test", slow_function);
    int result = timed_function();
    
    return 0;
}*/