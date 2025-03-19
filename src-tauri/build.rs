fn main() {
    /*
    println!("cargo:rerun-if-changed=../src-native/src/**/*");
    println!("cargo:rerun-if-changed=../src-native/CMakeLists.txt");
    println!("cargo:rerun-if-changed=../src-native/vcpkg-configuration.json");
    println!("cargo:rerun-if-changed=../src-native/vcpkg.json");

    let project_dir = std::env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR not set");
    let project_path = std::path::Path::new(&project_dir);

    // Build the C++ static library using CMake
    let dst = cmake::Config::new("../src-native")
        .generator("Visual Studio 17 2022")
        .no_default_flags(true)
        .configure_arg("-DGAME_MAPS_CORE_RELEASE=ON")
        .build_target("game-maps-core")
        .profile("Release")
        .build();

    let game_maps_core_dll_in = dst.join("build/Release/game-maps-core.dll").display().to_string();
    let game_maps_core_dll_out = project_path.join("game-maps-core.dll").display().to_string();

    std::fs::copy(game_maps_core_dll_in, game_maps_core_dll_out).unwrap();

    // Link the library
    println!("cargo:rustc-link-search=native={}/build/Release", dst.display());
    // println!("cargo:rustc-link-search=native={}/build/vcpkg_installed/x64-windows/lib", dst.display());
    println!("cargo:rustc-link-lib=game-maps-core");
    println!("cargo:rustc-link-lib=user32"); // Windows API dependency
    // println!("cargo:rustc-link-lib=static=leveldb");
    // println!("cargo:rustc-link-lib=static=libcurl");
    // println!("cargo:rustc-link-lib=static=nghttp2"); // libcurl
    // println!("cargo:rustc-link-lib=static=zlib"); // libcurl
    // println!("cargo:rustc-link-lib=static=zstd"); // libcurl
    // println!("cargo:rustc-link-lib=ws2_32"); // libcurl
    // println!("cargo:rustc-link-lib=bcrypt"); // libcurl
    // println!("cargo:rustc-link-lib=advapi32"); // libcurl
    // println!("cargo:rustc-link-lib=crypt32"); // libcurl

    // println!("cargo:rustc-link-search=native=C:/Projects/game-maps/src-tauri");
    // println!("cargo:rustc-link-lib=game-maps-core");
    */

    // tauri build
    tauri_build::build();
}
