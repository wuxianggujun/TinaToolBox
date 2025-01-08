mod foo; // 假设你仍然需要这个模块

use calamine::{open_workbook, Reader, Xlsx};

#[cxx::bridge(namespace = "lib")]
mod bridge {
    extern "Rust" {
        fn open_xlsx_file(path: &str) -> bool;
    }
}

fn open_xlsx_file(path: &str) -> bool {
    match open_workbook::<Xlsx<_>, _>(path) {
        Ok(_) => {
            println!("Successfully opened XLSX file: {}", path);
            true
        }
        Err(e) => {
            eprintln!("Error opening XLSX file: {}: {}", path, e);
            false
        }
    }
}