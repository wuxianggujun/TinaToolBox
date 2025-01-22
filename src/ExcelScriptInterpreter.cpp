//
// Created by wuxianggujun on 2025/1/21.
//

#include "ExcelScriptInterpreter.hpp"
#include "ExcelHandler.hpp"
#include <iostream>

namespace TinaToolBox
{
    ExcelScriptInterpreter::ExcelScriptInterpreter(std::shared_ptr<ExcelHandler> handler): excelHandler(handler)
    {
    }

    std::any ExcelScriptInterpreter::visitOpenStatement(ExcelScriptParser::OpenStatementContext* context)
    {
        std::string filename = context->STRING()->getText();
        filename = filename.substr(1, filename.length() - 2); // 移除引号
        if (excelHandler)
        {
            if (excelHandler->openFile(filename))
            {
                std::cout << "Successfully opened file: " << filename << std::endl;
            }
            else
            {
                std::cerr << "Failed to open file: " << filename << std::endl;
            }
        }
        return nullptr;
    }

    std::any ExcelScriptInterpreter::visitSelectSheetStatement(
        ExcelScriptParser::SelectSheetStatementContext* context)
    {
        if (!excelHandler) return nullptr;

        bool success = false;
        if (context->STRING())
        {
            std::string sheetName = context->STRING()->getText();
            sheetName = sheetName.substr(1, sheetName.length() - 2);
            success = excelHandler->selectSheet(sheetName);
            if (success)
            {
                std::cout << "Selected sheet: " << sheetName << std::endl;
            }
        }
        else if (context->NUMBER())
        {
            int sheetIndex = std::stoi(context->NUMBER()->getText());
            success = excelHandler->selectSheet(sheetIndex - 1); // 转换为0基索引
            if (success)
            {
                std::cout << "Selected sheet at index: " << sheetIndex << std::endl;
            }
        }

        if (!success)
        {
            std::cerr << "Failed to select sheet" << std::endl;
        }
        return nullptr;
    }

    std::any ExcelScriptInterpreter::visitReadCellStatement(ExcelScriptParser::ReadCellStatementContext* ctx)
    {
        if (!excelHandler) return nullptr;

        std::string cellRef = ctx->cell()->CELL_REF()->getText();
        std::string value = excelHandler->readCell(cellRef);
        std::cout << "Cell " << cellRef << " contains: " << value << std::endl;
        return value;
    }

    std::any ExcelScriptInterpreter::visitWriteCellStatement(ExcelScriptParser::WriteCellStatementContext* ctx)
    {
        if (!excelHandler) return nullptr;

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

        std::string cellRef = ctx->cell()->CELL_REF()->getText();
        if (excelHandler->writeCell(cellRef, value))
        {
            std::cout << "Successfully wrote value: " << value << " to cell: " << cellRef << std::endl;
            excelHandler->save(); // 自动保存更改
        }
        else
        {
            std::cerr << "Failed to write to cell: " << cellRef << std::endl;
        }
        return nullptr;
    }

    bool ExcelScriptInterpreter::executeScript(const std::string& script)
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
            visit(tree);
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error executing script: " << e.what() << std::endl;
            return false;
        }
    }
} // TinaToolBox
