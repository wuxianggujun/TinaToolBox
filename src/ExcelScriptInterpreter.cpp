//
// Created by wuxianggujun on 2025/1/21.
//

#include "ExcelScriptInterpreter.hpp"
#include "ExcelHandler.hpp"
#include <iostream>
#include <utility>
#include <filesystem>
#include <sstream>

namespace TinaToolBox
{
    ExcelScriptInterpreter::ExcelScriptInterpreter(std::shared_ptr<ExcelHandler> handler): excelHandler(std::move(handler))
    {
    }

    ExcelScriptInterpreter::~ExcelScriptInterpreter() = default;

    void ExcelScriptInterpreter::setConfig(const std::string& key, const std::string& value) {
        config_[key] = value;
    }

    std::string ExcelScriptInterpreter::getConfig(const std::string& key, const std::string& defaultValue) const {
        auto it = config_.find(key);
        return it != config_.end() ? it->second : defaultValue;
    }

    const std::map<std::string, std::string>& ExcelScriptInterpreter::getAllConfig() const {
        return config_;
    }

    void ExcelScriptInterpreter::setInitialConfig(const std::map<std::string, std::string>& config) {
        config_ = config;
    }

    std::any ExcelScriptInterpreter::visitOpenStatement(ExcelScriptParser::OpenStatementContext* context)
    {
        std::string filename;
        if (auto* configValue = context->value()->configValue()) {
            // 如果是配置值引用
            std::string configKey = configValue->STRING()->getText();
            configKey = configKey.substr(1, configKey.length() - 2); // 移除引号
            filename = getConfig(configKey);
        } else if (auto* stringValue = context->value()->STRING()) {
            // 如果是直接的字符串值
            filename = stringValue->getText();
            filename = filename.substr(1, filename.length() - 2); // 移除引号
        } else {
            lastError = "Invalid filename type";
            return ErrorCode::INVALID_VALUE;
        }
        
        // 检查文件是否存在
        if (!std::filesystem::exists(filename)) {
            lastError = "File not found: " + filename;
            return ErrorCode::FILE_NOT_FOUND;
        }

        if (excelHandler) {
            if (excelHandler->openFile(filename)) {
                std::cout << "Successfully opened file: " << filename << std::endl;
                return ErrorCode::SUCCESS;
            }
            lastError = "Failed to open file: " + filename;
            return ErrorCode::FILE_NOT_FOUND;
        }
        lastError = "Excel handler not initialized";
        return ErrorCode::EXECUTION_ERROR;
    }

    std::any ExcelScriptInterpreter::visitSelectSheetStatement(ExcelScriptParser::SelectSheetStatementContext* context)
    {
        if (!excelHandler) {
            lastError = "Excel handler not initialized";
            return ErrorCode::EXECUTION_ERROR;
        }

        bool success = false;
        if (auto* configValue = context->value()->configValue()) {
            // 如果是配置值引用
            std::string configKey = configValue->STRING()->getText();
            configKey = configKey.substr(1, configKey.length() - 2); // 移除引号
            std::string sheetName = getConfig(configKey);
            success = excelHandler->selectSheet(sheetName);
            if (success) {
                std::cout << "Selected sheet: " << sheetName << std::endl;
            } else {
                lastError = "Sheet not found: " + sheetName;
                return ErrorCode::SHEET_NOT_FOUND;
            }
        } else if (auto* stringValue = context->value()->STRING()) {
            // 如果是字符串值
            std::string sheetName = stringValue->getText();
            sheetName = sheetName.substr(1, sheetName.length() - 2); // 移除引号
            success = excelHandler->selectSheet(sheetName);
            if (success) {
                std::cout << "Selected sheet: " << sheetName << std::endl;
            } else {
                lastError = "Sheet not found: " + sheetName;
                return ErrorCode::SHEET_NOT_FOUND;
            }
        } else if (auto* numberValue = context->value()->NUMBER()) {
            // 如果是数字值
            int sheetIndex = std::stoi(numberValue->getText());
            success = excelHandler->selectSheet(sheetIndex - 1); // 转换为0基索引
            if (success) {
                std::cout << "Selected sheet at index: " << sheetIndex << std::endl;
            } else {
                lastError = "Invalid sheet index: " + std::to_string(sheetIndex);
                return ErrorCode::SHEET_NOT_FOUND;
            }
        }

        return success ? ErrorCode::SUCCESS : ErrorCode::SHEET_NOT_FOUND;
    }

    std::any ExcelScriptInterpreter::visitReadCellStatement(ExcelScriptParser::ReadCellStatementContext* ctx)
    {
        if (!excelHandler) {
            lastError = "Excel handler not initialized";
            return ErrorCode::EXECUTION_ERROR;
        }

        std::string cellRef = ctx->cell()->CELL_REF()->getText();
        std::string value = excelHandler->readCell(cellRef);
        if (value.empty()) {
            lastError = "Failed to read cell: " + cellRef;
            return ErrorCode::CELL_ACCESS_ERROR;
        }
        std::cout << "Cell " << cellRef << " contains: " << value << std::endl;
        return ErrorCode::SUCCESS;
    }

    std::any ExcelScriptInterpreter::visitWriteCellStatement(ExcelScriptParser::WriteCellStatementContext* ctx)
    {
        if (!excelHandler) {
            lastError = "Excel handler not initialized";
            return ErrorCode::EXECUTION_ERROR;
        }

        std::string value;
        if (ctx->value()->STRING())
        {
            value = ctx->value()->STRING()->getText();
            value = value.substr(1, value.length() - 2);
        }
        else if (ctx->value()->NUMBER())
        {
            value = ctx->value()->NUMBER()->getText();
        }
        else {
            lastError = "Invalid value type";
            return ErrorCode::INVALID_VALUE;
        }

        std::string cellRef = ctx->cell()->CELL_REF()->getText();
        if (excelHandler->writeCell(cellRef, value))
        {
            std::cout << "Successfully wrote value: " << value << " to cell: " << cellRef << std::endl;
            excelHandler->save(); // 自动保存更改
            return ErrorCode::SUCCESS;
        }
        else
        {
            lastError = "Failed to write to cell: " + cellRef;
            return ErrorCode::CELL_ACCESS_ERROR;
        }
    }

    ExcelScriptInterpreter::ErrorCode ExcelScriptInterpreter::executeScript(const std::string& script)
    {
        try
        {
            // 创建输入流
            antlr4::ANTLRInputStream input(script);

            // 创建词法分析器
            ExcelScriptLexer lexer(&input);
            antlr4::CommonTokenStream tokens(&lexer);

            // 创建语法分析器
            ExcelScriptParser parser(&tokens);
            auto tree = parser.program();
            
            // 获取所有语句
            auto statements = tree->statement();
            
            // 依次执行每个语句
            for (auto* stmt : statements) {
                auto result = visit(stmt);
                
                // 检查执行结果
                if (result.has_value() && result.type() == typeid(ErrorCode)) {
                    auto errorCode = std::any_cast<ErrorCode>(result);
                    if (errorCode != ErrorCode::SUCCESS) {
                        return errorCode;  // 如果有任何语句执行失败，立即返回错误
                    }
                }
            }
            
            // 处理配置相关命令
            if (script.find("get config") != std::string::npos) {
                size_t keyStart = script.find('"', 11);  // 跳过"get config "
                size_t keyEnd = script.find('"', keyStart + 1);
                std::string key = script.substr(keyStart + 1, keyEnd - keyStart - 1);
                
                std::string value = getConfig(key);
                std::cout << "Config " << key << " = " << value << std::endl;
            }
            else if (script.find("set config") != std::string::npos) {
                size_t keyStart = script.find('"', 11);  // 跳过"set config "
                size_t keyEnd = script.find('"', keyStart + 1);
                std::string key = script.substr(keyStart + 1, keyEnd - keyStart - 1);
                
                size_t valueStart = script.find('"', keyEnd + 1);
                size_t valueEnd = script.find('"', valueStart + 1);
                std::string value = script.substr(valueStart + 1, valueEnd - valueStart - 1);
                
                setConfig(key, value);
            }
            
            return ErrorCode::SUCCESS;
        }
        catch (const std::exception& e)
        {
            lastError = std::string("Error executing script: ") + e.what();
            return ErrorCode::PARSE_ERROR;
        }
    }
} // TinaToolBox
