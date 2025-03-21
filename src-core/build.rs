fn main() {
    println!("cargo:rerun-if-changed=../src-native/include/*");

    let bindings = bindgen::Builder::default()
        .header("../src-native/include/network.h")
        .header("../src-native/include/app.h")
        .no_copy(".*")
        .generate()
        .expect("Unable to generate bindings");

    bindings
        .write_to_file("src/gen/bindings.rs")
        .expect("Couldn't write bindings!");
}