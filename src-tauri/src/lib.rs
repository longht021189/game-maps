use tauri::RunEvent;

// extern "C" {
//     fn core_start();
//     fn core_end();
// }

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    let host = if cfg!(dev) {
        "http://localhost:3000"
    } else {
        "https://mapgenie.io"
    };

    println!(">>>>>>>>>> host={}", host);

    tauri::Builder::default()
        .setup(|app| {
            #[cfg(dev)]
            {
                // let window = app.get_webview_window("main").unwrap();
                // window.open_devtools();
            }

            Ok(())
        })
        .plugin(tauri_plugin_opener::init())
        .build(tauri::generate_context!())
        .expect("error while build tauri application")
        .run(move |_app_handle, event| {
            // match event {
            //     RunEvent::Ready => {
            //         unsafe {
            //             core_start();
            //         }
            //     }
            //     RunEvent::ExitRequested { .. } => {
            //         unsafe {
            //             core_end();
            //         }
            //     }
            //     _ => {}
            // }
        });
}
