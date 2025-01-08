#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "rust/cxx.h"

namespace calamine {

struct ExcelData {
    rust::Vec<rust::String> data;
    rust::String error;
};

// 非安全版本，直接抛出 std::panic_error
std::vector<std::string> read_excel_sheet(const rust::Slice<uint8_t>& data, const rust::Str& sheet_name);

// 安全版本，通过 ExcelData 对象返回数据或错误信息
ExcelData read_excel_sheet_safe(const rust::Slice<uint8_t>& data, const rust::Str& sheet_name);

}