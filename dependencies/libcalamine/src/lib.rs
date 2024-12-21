//! This crate is used to generate `libextism` using `extism-runtime`



fn test_version() {
    let s = unsafe { std::ffi::CStr::from_ptr("Hello") };
    assert!(s.to_bytes() != b"0.0.0");
}