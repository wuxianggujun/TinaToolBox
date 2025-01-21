//
// Created by wuxianggujun on 2025/1/21.
//
#include <antlr4-runtime.h>
#include "ExcelScriptInterpreter.hpp"

namespace TinaToolBox {
    std::any ExcelScriptInterpreter::visitOpenStatement(excel_script::ExcelScriptParser::OpenStatementContext* context)
    {
        return ExcelScriptBaseVisitor::visitOpenStatement(context);
    }

    std::any ExcelScriptInterpreter::visitSelectSheetStatement(
        excel_script::ExcelScriptParser::SelectSheetStatementContext* context)
    {
        return ExcelScriptBaseVisitor::visitSelectSheetStatement(context);
    }

    std::any ExcelScriptInterpreter::visitReadCellStatement(
        excel_script::ExcelScriptParser::ReadCellStatementContext* ctx)
    {
        return ExcelScriptBaseVisitor::visitReadCellStatement(ctx);
    }

    std::any ExcelScriptInterpreter::visitWriteCellStatement(
        excel_script::ExcelScriptParser::WriteCellStatementContext* ctx)
    {
        return ExcelScriptBaseVisitor::visitWriteCellStatement(ctx);
    }
} // TinaToolBox