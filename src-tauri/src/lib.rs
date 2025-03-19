use tauri::RunEvent;

// extern "C" {
//     fn core_start();
//     fn core_end();
// }

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    println!(">>>>>>>>>>>>>>>>>> Hello, world!");

    tauri::Builder::default()
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
