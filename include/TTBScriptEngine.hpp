#pragma once

#include <string>
#include <memory>
#include <map>
#include <functional>
#include "TTBFile.hpp"
#include "ExcelScriptInterpreter.hpp"
#include "ExcelHandler.hpp"

namespace TinaToolBox
{
    class TTBScriptEngine {
    public:
        // 错误码
        enum class Error {
            SUCCESS,
            FILE_CREATE_ERROR,
            FILE_LOAD_ERROR,
            SCRIPT_EXECUTION_ERROR,
            INVALID_CONFIG
        };

        // 配置更新回调
        using ConfigUpdateCallback = std::function<void(const std::map<std::string, std::string>&)>;
        // 脚本执行进度回调
        using ProgressCallback = std::function<void(const std::string&, int)>;

        TTBScriptEngine();
        ~TTBScriptEngine();

        // 创建新的TTB文件
        Error createScript(const std::string& filename,
                          const std::map<std::string, std::string>& config,
                          const std::string& script,
                          bool encrypt = true);

        // 加载并执行TTB文件
        Error executeScript(const std::string& filename,
                          const AESKey& key = AESKey());

        // 加载并执行TTB文件 (通过 TTBFile 对象)  <--- 添加这个重载
        Error executeScript(const std::string& filename, // 为了保持接口一致，可以保留 filename 参数，即使在这里可能不直接使用
                          const AESKey& key,
                          TTBFile* ttbFile);

        // 获取最后一次错误信息
        std::string getLastError() const;

        // 设置配置更新回调
        void setConfigUpdateCallback(ConfigUpdateCallback callback);

        // 设置进度回调
        void setProgressCallback(ProgressCallback callback);

        // 获取当前配置
        const std::map<std::string, std::string>& getCurrentConfig() const;

        // 验证TTB文件
        bool validateTTBFile(const std::string& filename) const;

    private:
        class Impl;
        std::unique_ptr<Impl> pimpl;
    };
}