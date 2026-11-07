//! Initialization scripts built by Rsbuild (`pnpm build:inject`, entries in `src/entrypoint/`,
//! environment `inject`) into `dist-inject/`, and embedded here at compile time.

use tauri::{
    plugin::{Builder, TauriPlugin},
    Runtime,
};

/// Overrides `fetch` / `XMLHttpRequest` (every platform) and hands request bodies to wry's
/// interceptor (Android), see `src/entrypoint/http-bodies.ts`.
const HTTP_BODIES: &str = include_str!(concat!(
    env!("CARGO_MANIFEST_DIR"),
    "/../dist-inject/http-bodies.js"
));

/// Registers the scripts in every frame (the map is an `<iframe>`).
pub fn plugin<R: Runtime>() -> TauriPlugin<R> {
    Builder::new("inject")
        .js_init_script_on_all_frames(HTTP_BODIES)
        .build()
}
