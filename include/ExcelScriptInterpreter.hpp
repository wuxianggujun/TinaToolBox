//
// Created by wuxianggujun on 2025/1/21.
//

#ifndef EXCEL_SCRIPT_INTERPRETER_H
#define EXCEL_SCRIPT_INTERPRETER_H

#pragma push_macro("ERROR")
#pragma push_macro("emit")
#undef ERROR
#undef emit

#include <ExcelScriptParser.h>
#include <ExcelScriptBaseVisitor.h>
#include <antlr4-runtime.h>
#include <ExcelScriptLexer.h>
#include <ExcelScriptParser.h>
#include <memory>
#include <string>
#include <map>
#include "ExcelHandler.hpp"

namespace TinaToolBox
{
    class ExcelHandler;

    class ExcelScriptInterpreter : public ExcelScriptBaseVisitor
    {
    public:
        // 定义错误码枚举
        enum class ErrorCode {
            SUCCESS = 0,
            FILE_NOT_FOUND = 1,
            SHEET_NOT_FOUND = 2,
            CELL_ACCESS_ERROR = 3,
            INVALID_VALUE = 4,
            PARSE_ERROR = 5,
            EXECUTION_ERROR = 6,
            SYNTAX_ERROR = 7,
            RUNTIME_ERROR = 8,
            FILE_ERROR = 9,
            CONFIG_ERROR = 10
        };

        ExcelScriptInterpreter(std::shared_ptr<ExcelHandler> excelHandler);
        ~ExcelScriptInterpreter();
        
        // 设置Excel处理器
        void setExcelHandler(std::shared_ptr<ExcelHandler> handler) { excelHandler = handler; }
        
        // 访问器方法
        std::any visitOpenStatement(ExcelScriptParser::OpenStatementContext* context) override;
        std::any visitSelectSheetStatement(
            ExcelScriptParser::SelectSheetStatementContext* context) override;
        std::any visitReadCellStatement(ExcelScriptParser::ReadCellStatementContext* ctx) override;
        std::any visitWriteCellStatement(ExcelScriptParser::WriteCellStatementContext* ctx) override;
        
        // 执行脚本
        ErrorCode executeScript(const std::string& script);

        // 获取最后一次错误信息
        const std::string& getLastError() const { return lastError; }

        // 新增：配置相关方法
        void setConfig(const std::string& key, const std::string& value);
        std::string getConfig(const std::string& key, const std::string& defaultValue = "") const;
        const std::map<std::string, std::string>& getAllConfig() const;
        void setInitialConfig(const std::map<std::string, std::string>& config);

    private:
        std::shared_ptr<ExcelHandler> excelHandler;
        std::string lastError;  // 存储最后一次错误信息
        std::map<std::string, std::string> config_;  // 配置存储
    };
} // TinaToolBox

#endif //EXCEL_SCRIPT_INTERPRETER_H
#pragma pop_macro("ERROR")
#pragma pop_macro("emit")
