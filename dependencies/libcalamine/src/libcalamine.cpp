#include "./include/libcalamine.h"

namespace calamine {

ExcelData read_excel_sheet_safe(const rust::Slice<uint8_t>& data, const rust::Str& sheet_name) {
    return ::calamine::read_excel_sheet_safe(data, sheet_name);
}

std::vector<std::string> read_excel_sheet(const rust::Slice<uint8_t>& data, const rust::Str& sheet_name){
    return ::calamine::read_excel_sheet(data, sheet_name);
}
}