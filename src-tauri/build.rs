fn main() {
    // `src/inject.rs` embeds the Rsbuild output: fail early with a clear message.
    let script = "../dist-inject/http-bodies.js";
    println!("cargo:rerun-if-changed={script}");
    if !std::path::Path::new(script).exists() {
        panic!("`{script}` is missing: run `pnpm build:inject` (done by `beforeDevCommand` / `beforeBuildCommand`)");
    }

    tauri_build::build()
}
