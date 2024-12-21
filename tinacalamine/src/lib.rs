#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("tinacalamine/include/CalamineWrapper.hpp");

        type CalamineWrapper;

        fn new_calamine() -> cxx::UniquePtr<CalamineWrapper> {
            unsafe { new_calamine_impl() }
        }
        fn read_a_value(&self) -> i32; // 注意这里被隐式推断为 const
    }
}

// Rust 实现部分，可以留空，因为你主要在 C++ 中使用 Calamine
pub struct RustImpl;

impl RustImpl {
    pub fn new() -> Self {
        RustImpl
    }
}