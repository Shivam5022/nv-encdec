#![allow(warnings)]

// use bindgen;
use std::env;
use std::path::Path;
use std::process::Command;

const LIB_NAME: &str = "NvEncDec";

fn main() {
    // Define paths to the C++ source and build directories
    // let out_dir = env::var("OUT_DIR").unwrap();
    let cpp_src_dir = Path::new(&env::var("CARGO_MANIFEST_DIR").unwrap()).join("cpp");
    let build_dir = Path::new(&cpp_src_dir).join("cpp_build");

    // Create the build directory if it doesn't exist
    std::fs::create_dir_all(&build_dir).unwrap();

    // Build the C++ shared library using CMake
    println!("Running CMake to build C++ code...");
    let status = Command::new("cmake")
        .arg("-S")
        .arg(cpp_src_dir.clone())
        .arg("-B")
        .arg(&build_dir)
        .arg("-DCMAKE_BUILD_TYPE=Release")
        .arg("-DCMAKE_INSTALL_PREFIX=")
        .status()
        .unwrap();
    assert!(status.success(), "CMake configuration failed");

    let status = Command::new("cmake")
        .arg("--build")
        .arg(&build_dir)
        .arg("--target")
        .arg(LIB_NAME)
        .arg("--config")
        .arg("Release")
        .status()
        .unwrap();
    assert!(status.success(), "CMake build failed");

    // Add the directory containing the shared library to the search path
    println!("cargo:rustc-link-search=native={}", build_dir.display());

    // Link the library (assuming it is named libNvEncDec.so)
    println!("cargo:rustc-link-lib=dylib={}", LIB_NAME);

    // Use bindgen to generate Rust bindings from the C++ headers
    println!("Generating Rust bindings using bindgen...");

    let bindings = bindgen::Builder::default()
        .header(
            cpp_src_dir
                .clone()
                .join("include")
                .join("DecHeader.h")
                .to_str()
                .unwrap(),
        )
        .clang_arg("-x")
        .clang_arg("c++")
        .clang_arg("-std=c++11")
        .derive_default(false)
        .generate()
        .expect("Unable to generate bindings");
    // let bindings = bindgen::Builder::default()
    //     .header(
    //         cpp_src_dir
    //             .join("include")
    //             .join("DecHeader.h")
    //             .to_str()
    //             .unwrap(),
    //     ) // Replace with your actual header file location
    //     .parse_callbacks(Box::new(bindgen::CargoCallbacks))
    //     .generate()
    //     .expect("Unable to generate bindings");

    // Write the bindings to a file in the OUT_DIR
    let rust_src_dir = Path::new(&env::var("CARGO_MANIFEST_DIR").unwrap()).join("src");
    let bindings_out_path = Path::new(&rust_src_dir).join("nvencdec.rs");
    bindings
        .write_to_file(bindings_out_path)
        .expect("Couldn't write bindings!");

    println!("cargo:rerun-if-changed=cpp/include/*.h");
    println!("cargo:rerun-if-changed=cpp/src/*.cpp");
}
