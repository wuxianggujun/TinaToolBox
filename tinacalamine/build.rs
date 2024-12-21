fn main() {
    cxx_build::bridge("src/lib.rs")
        .flag_if_supported("-std=c++17")
        .compile("tinacalamine");

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=include/CalamineWrapper.hpp");
}