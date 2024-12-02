//
// Created by wuxianggujun on 2024/11/23.
//

#ifndef TINA_TOOL_BOX_EXCEPTION_HANDLER_HPP
#define TINA_TOOL_BOX_EXCEPTION_HANDLER_HPP

#include <functional>
#include <string>
#include <exception>
#include <QSqlDatabase>
#include <QSqlError>
#include <utility>
#include <spdlog/spdlog.h>

class ExceptionHandler {
public:
    explicit ExceptionHandler(std::string errorMessage = "程序异常",
                              spdlog::level::level_enum logLevel = spdlog::level::err) : errorMessage_(std::move(errorMessage)),
        logLevel_(logLevel) {
    }

    virtual ~ExceptionHandler() = default;

    template<class Func, class... Args>
    auto operator()(Func &&func, Args &&... args) -> decltype(func(std::forward<Args>(args)...)) {
        try {
            // 直接执行函数，不再使用全局标志
            return func(std::forward<Args>(args)...);
        } catch (const std::invalid_argument &e) {
            // 处理参数验证异常
            std::string error = errorMessage_ + ": " + e.what();
            spdlog::log(logLevel_, error);
            return {};
        } catch (const std::exception &e) {
            // 处理其他标准异常
            spdlog::log(logLevel_, "{}: {}", errorMessage_, e.what());
            return {};
        } catch (...) {
            // 处理未知异常
            spdlog::log(logLevel_, "{}: 未知异常", errorMessage_);
            return {};
        }
    }

protected:
    std::string errorMessage_;
    spdlog::level::level_enum logLevel_ = spdlog::level::info;
};

// 数据库异常处理器
class DBExceptionHandler : public ExceptionHandler {
public:
    explicit DBExceptionHandler(
        const std::string& errorMessage = "数据库操作失败",
        bool autoRollback = true
    ) : ExceptionHandler(errorMessage), autoRollback_(autoRollback) {}

    template<typename Func, typename... Args>
    auto operator()(Func&& func, Args&&... args) -> decltype(func(std::forward<Args>(args)...)) {
        try {
            return func(std::forward<Args>(args)...);
        } catch (const std::invalid_argument& e) {
            spdlog::error("{}: {}", errorMessage_, e.what());
            handleRollback();
            return {};
        } catch (const QSqlError& e) {
            spdlog::error("数据库异常: {}", e.text().toStdString());
            handleRollback();
            return {};
        } catch (const std::exception& e) {
            spdlog::error("数据库操作异常: {}", e.what());
            handleRollback();
            return {};
        } catch (...) {
            spdlog::error("数据库操作发生未知异常");
            handleRollback();
            return {};
        }
    }

private:
    bool autoRollback_;

    void handleRollback() const {
        if (autoRollback_) {
            QSqlDatabase db = QSqlDatabase::database();
            if (db.isValid() && db.transaction()) {
                db.rollback();
            }
        }
    }
};

#endif //TINA_TOOL_BOX_EXCEPTION_HANDLER_HPP
