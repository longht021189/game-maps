use tauri::{Manager, RunEvent};

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    if cfg!(dev) {
        let project_dir = std::env::current_dir()
            .unwrap()
            .to_string_lossy()
            .into_owned() + "/../dist";

        unsafe {
            game_maps_core::setup(project_dir.as_str());
        }
    }

    tauri::Builder::default()
        .setup(|app| {
            #[cfg(dev)]
            {
                let window = app.get_webview_window("main").unwrap();
                window.open_devtools();
            }

            Ok(())
        })
        .plugin(tauri_plugin_opener::init())
        .run(tauri::generate_context!())
        .expect("error while run tauri application");
}
