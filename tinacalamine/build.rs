fn main() {
    cxx_build::bridge("src/main.rs")
        .file("src/blobstore.cc")
        .std("c++17")
        .compile("tinacalamine");

    println!("cargo:rerun-if-changed=src/main.rs");
    println!("cargo:rerun-if-changed=src/blobstore.cc");
    println!("cargo:rerun-if-changed=include/blobstore.h");
}