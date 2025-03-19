fn main() {
    println!("cargo:rerun-if-changed=../src-native/src/main.h");

    let bindings = bindgen::Builder::default()
        .header("../src-native/src/main.h")
        .generate()
        .expect("Unable to generate bindings");

    bindings
        .write_to_file("src/gen/bindings.rs")
        .expect("Couldn't write bindings!");
}