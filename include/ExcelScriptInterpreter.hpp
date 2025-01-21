//
// Created by wuxianggujun on 2025/1/21.
//



#ifndef EXCEL_SCRIPT_INTERPRETER_H
#define EXCEL_SCRIPT_INTERPRETER_H

#pragma push_macro("ERROR")
#pragma push_macro("emit")
#undef ERROR
#undef emit

// 你的代码

// 在头文件结束处
#include <ExcelScriptLexer.h>
#include <ExcelScriptParser.h>
#include <ExcelScriptBaseVisitor.h>

// 假设你有一个名为ExcelHandler的类来处理Excel操作
//#include "ExcelHandler.h"

namespace TinaToolBox
{
    class ExcelHandler;

    class ExcelScriptInterpreter : public excel_script::ExcelScriptBaseVisitor
    {
    public:
        // ExcelHandler excelHandler;

        std::any visitOpenStatement(excel_script::ExcelScriptParser::OpenStatementContext* context) override;
        std::any visitSelectSheetStatement(
            excel_script::ExcelScriptParser::SelectSheetStatementContext* context) override;
        std::any visitReadCellStatement(excel_script::ExcelScriptParser::ReadCellStatementContext* ctx) override;
        std::any visitWriteCellStatement(excel_script::ExcelScriptParser::WriteCellStatementContext* ctx) override;
    };
} // TinaToolBox

#endif //EXCEL_SCRIPT_INTERPRETER_H

#pragma pop_macro("ERROR")
#pragma pop_macro("emit")