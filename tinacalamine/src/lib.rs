use calamine::{open_workbook, Xlsx};
use cxx::{type_id, ExternType};
use std::collections::HashMap;
use std::sync::{Arc, RwLock};
use std::fs::File;
use std::io::BufReader;

type WorkbookStore = Arc<RwLock<HashMap<FileHandle, Xlsx<BufReader<File>>>>>;

lazy_static::lazy_static! {
    static ref WORKBOOKS: WorkbookStore = Arc::new(RwLock::new(HashMap::new()));
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
struct FileHandle(usize);

unsafe impl ExternType for FileHandle {
    type Id = type_id!("tinacalamine::FileHandle");
    type Kind = cxx::kind::Trivial;
}

#[cxx::bridge(namespace = "ffi")]
mod ffi {
    unsafe extern "C++" {
        include!("CalamineWrapper.hpp");
        fn openFile(path: &str) -> Box<FileHandle>;
    }

    extern "Rust" {
        type FileHandle;
        fn rust_open_file(path: &str) -> Box<FileHandle>;
    }
}

fn rust_open_file(path: &str) -> Box<FileHandle> {
    match open_workbook::<Xlsx<_>, _>(path) {
        Ok(workbook) => {
            let mut workbooks = WORKBOOKS.write().unwrap();
            let handle = generate_handle();
            workbooks.insert(FileHandle(handle), workbook);
            Box::new(FileHandle(handle))
        }
        Err(e) => {
            // 使用 eprintln! 将错误信息打印到标准错误流
            eprintln!("Failed to open file '{}': {}", path, e);
            // 返回一个表示错误的空指针
            Box::new(FileHandle(0))
        }
    }
}

fn generate_handle() -> usize {
    use std::sync::atomic::{AtomicUsize, Ordering};
    static COUNTER: AtomicUsize = AtomicUsize::new(1);
    COUNTER.fetch_add(1, Ordering::Relaxed)
}