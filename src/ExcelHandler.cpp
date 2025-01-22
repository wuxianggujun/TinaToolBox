#include "ExcelHandler.hpp"
#include <xlnt/xlnt.hpp>
#include <iostream>

namespace TinaToolBox {

class ExcelHandler::Impl {
public:
    xlnt::workbook workbook;
    xlnt::worksheet current_worksheet;
    bool is_open = false;
    std::string current_filename;  // 添加文件名存储

    bool openWorksheet(const std::string& sheetName) {
        try {
            current_worksheet = workbook.sheet_by_title(sheetName);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error opening worksheet: " << e.what() << std::endl;
            return false;
        }
    }

    bool openWorksheet(int index) {
        try {
            current_worksheet = workbook.sheet_by_index(index);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error opening worksheet: " << e.what() << std::endl;
            return false;
        }
    }
};

ExcelHandler::ExcelHandler() : pimpl(std::make_unique<Impl>()) {}
ExcelHandler::~ExcelHandler() = default;

bool ExcelHandler::openFile(const std::string& filename) {
    try {
        pimpl->workbook.load(filename);
        pimpl->is_open = true;
        pimpl->current_filename = filename;  // 保存文件名
        // 默认选择第一个工作表
        if (!pimpl->workbook.sheet_titles().empty()) {
            pimpl->current_worksheet = pimpl->workbook.active_sheet();
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error opening Excel file: " << e.what() << std::endl;
        return false;
    }
}

bool ExcelHandler::selectSheet(const std::string& sheetName) {
    if (!pimpl->is_open) return false;
    return pimpl->openWorksheet(sheetName);
}

bool ExcelHandler::selectSheet(int sheetIndex) {
    if (!pimpl->is_open) return false;
    return pimpl->openWorksheet(sheetIndex);
}

std::string ExcelHandler::readCell(const std::string& cellRef) {
    if (!pimpl->is_open) return "";
    try {
        xlnt::cell cell = pimpl->current_worksheet.cell(cellRef);
        return cell.to_string();
    } catch (const std::exception& e) {
        std::cerr << "Error reading cell: " << e.what() << std::endl;
        return "";
    }
}

bool ExcelHandler::writeCell(const std::string& cellRef, const std::string& value) {
    if (!pimpl->is_open) return false;
    try {
        xlnt::cell cell = pimpl->current_worksheet.cell(cellRef);
        cell.value(value);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing cell: " << e.what() << std::endl;
        return false;
    }
}

bool ExcelHandler::save() {
    if (!pimpl->is_open || pimpl->current_filename.empty()) return false;
    try {
        pimpl->workbook.save(pimpl->current_filename);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving workbook: " << e.what() << std::endl;
        return false;
    }
}

} // TinaToolBox 