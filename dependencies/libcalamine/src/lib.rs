use calamine::{open_workbook, Xlsx, Reader, DataType};
use cxx::CxxString;
use std::mem;

#[cxx::bridge(namespace = "lib")]
mod bridge {
    struct SpreadsheetData {
        sheets: Vec<SpreadsheetSheet>,
    }

    struct SpreadsheetSheet {
        name: String,
        rows: *mut CxxString,
        rows_len: usize,
    }

    extern "Rust" {
        fn read_xlsx_custom(path: &str) -> SpreadsheetData;
        unsafe fn free_spreadsheet_data(data: SpreadsheetData);
    }
}

fn read_xlsx_custom(path: &str) -> bridge::SpreadsheetData {
    let mut spreadsheet_data = bridge::SpreadsheetData { sheets: Vec::new() };
    match open_workbook::<Xlsx<_>, _>(path) {
        Ok(workbook) => {
            for sheet_name in workbook.sheet_names() {
                if let Some(Ok(range)) = workbook.worksheet_range(&sheet_name) {
                    let mut all_rows_cxx_strings: Vec<CxxString> = Vec::new();
                    for row in range.rows() {
                        for cell in row {
                            let cxx_string = match cell {
                                DataType::String(s) => CxxString::from(s.clone()),
                                DataType::Float(f) => CxxString::from(f.to_string()),
                                DataType::Int(i) => CxxString::from(i.to_string()),
                                DataType::Bool(b) => CxxString::from(b.to_string()),
                                DataType::Error(e) => CxxString::from(format!("{:?}", e)),
                                DataType::Empty => CxxString::from(""),
                            };
                            all_rows_cxx_strings.push(cxx_string);
                        }
                    }
                    let rows_len = all_rows_cxx_strings.len();
                    let rows_ptr = if rows_len > 0{
                        all_rows_cxx_strings.as_mut_ptr()
                    }else{
                        std::ptr::null_mut()
                    };
                    mem::forget(all_rows_cxx_strings);
                    let sheet_data = bridge::SpreadsheetSheet {
                        name: sheet_name.clone(),
                        rows: rows_ptr,
                        rows_len,
                    };
                    spreadsheet_data.sheets.push(sheet_data);
                }else{
                    eprintln!("Error reading sheet: {}", sheet_name);
                }
            }
        }
        Err(e) => eprintln!("Error opening workbook: {}", e),
    }
    spreadsheet_data
}

unsafe fn free_spreadsheet_data(data: bridge::SpreadsheetData) {
    for sheet in data.sheets {
        if !sheet.rows.is_null() {
            let _ = Vec::from_raw_parts(sheet.rows, sheet.rows_len, sheet.rows_len);
        }
    }
}