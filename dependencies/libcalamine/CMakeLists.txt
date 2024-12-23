# --- 导入 Rust 库 ---
corrosion_import_crate(MANIFEST_PATH ./Cargo.toml)
corrosion_add_cxxbridge(cxxbridge-cpp CRATE cxxbridge_crate MANIFEST_PATH rust FILES lib.rs foo/mod.rs)