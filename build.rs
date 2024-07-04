// build.rs
use std::env;
use std::path::PathBuf;

fn main() {
    cc::Build::new()
        .file("c_src/wakeonlan.c")
        .compile("wakeonlan");

    println!("cargo:rerun-if-changed=c_src/wakeonlan.c");

    let bindings = bindgen::Builder::default()
        .header("c_src/wakeonlan.h")
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
