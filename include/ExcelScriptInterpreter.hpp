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

namespace TinaToolBox
{
    class ExcelHandler;

    class ExcelScriptInterpreter : public ExcelScriptBaseVisitor
    {
    public:
        explicit ExcelScriptInterpreter(std::shared_ptr<ExcelHandler> handler = nullptr);
        
        // 设置Excel处理器
        void setExcelHandler(std::shared_ptr<ExcelHandler> handler) { excelHandler = handler; }
        
        // 访问器方法
        std::any visitOpenStatement(ExcelScriptParser::OpenStatementContext* context) override;
        std::any visitSelectSheetStatement(
            ExcelScriptParser::SelectSheetStatementContext* context) override;
        std::any visitReadCellStatement(ExcelScriptParser::ReadCellStatementContext* ctx) override;
        std::any visitWriteCellStatement(ExcelScriptParser::WriteCellStatementContext* ctx) override;
        
        // 执行脚本
        bool executeScript(const std::string& script);

    private:
        std::shared_ptr<ExcelHandler> excelHandler;
    };
} // TinaToolBox

#endif //EXCEL_SCRIPT_INTERPRETER_H
#pragma pop_macro("ERROR")
#pragma pop_macro("emit")
