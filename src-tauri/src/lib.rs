use tauri::{Manager, RunEvent};

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    let host = if cfg!(dev) {
        "http://localhost:3000"
    } else {
        "https://mapgenie.io"
    };

    println!(">>>>>>>>>> host={}", host);

    let project_dir = std::env::current_dir()
        // .map_err(|e| format!("Failed to get current dir: {}", e))
        .unwrap()
        .to_string_lossy()
        .into_owned();

    println!(">>>>>>>>>> project_dir={}", project_dir);

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
