fn main() {
    if std::env::var("TARGET").unwrap().contains("msvc") {
        println!("cargo:rustc-link-arg=/MDd"); // 添加 /MDd 链接选项
        println!("cargo:rustc-link-arg=/DEBUG"); // 添加 /DEBUG 链接选项，为了生成调试信息
    }
}