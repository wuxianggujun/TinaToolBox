#ifndef EXCEL_HANDLER_H
#define EXCEL_HANDLER_H

#include <string>
#include <memory>

namespace TinaToolBox {

class ExcelHandler {
public:
    ExcelHandler();
    ~ExcelHandler();

    // Excel 操作接口
    bool openFile(const std::string& filename);
    bool selectSheet(const std::string& sheetName);
    bool selectSheet(int sheetIndex);
    std::string readCell(const std::string& cellRef);
    bool writeCell(const std::string& cellRef, const std::string& value);
    bool save();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // TinaToolBox

#endif // EXCEL_HANDLER_H 