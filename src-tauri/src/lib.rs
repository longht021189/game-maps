mod inject;
mod locations;
mod proxy;

use std::sync::Arc;

use tauri::Manager;

use locations::LocationStore;

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    if !wry::is_http_interception_supported() {
        eprintln!("error: this platform's WebView cannot intercept http(s) requests");
        std::process::exit(1);
    }

    tauri::Builder::default()
        .plugin(inject::plugin())
        .setup(|app| {
            // Found locations of mapgenie maps, see `proxy::user_locations`.
            let dir = app.path().app_data_dir()?;
            std::fs::create_dir_all(&dir)?;
            let store = LocationStore::open(&dir.join("locations.redb"))
                .map_err(|err| -> Box<dyn std::error::Error> { err })?;
            app.manage(Arc::new(store));
            Ok(())
        })
        // The wry fork turns custom protocols named after an origin into HTTP
        // interceptors: these two route every http(s) request through the proxy.
        .register_asynchronous_uri_scheme_protocol(wry::ANY_HTTP_ORIGIN, proxy::protocol)
        .register_asynchronous_uri_scheme_protocol(wry::ANY_HTTPS_ORIGIN, proxy::protocol)
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
