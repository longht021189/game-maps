use std::fmt::Debug;
use webview2_com::Microsoft::Web::WebView2::Win32::{
    ICoreWebView2,
    ICoreWebView2WebResourceRequest,
    COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL
};
use windows::core::HSTRING;

mod types;
pub use types::*;

mod native;
mod request;
mod response;

#[inline]
pub fn is_network_override() -> bool {
    true
}

#[inline]
pub unsafe fn add_web_resource_requested_filter(
    webview: &ICoreWebView2
) -> windows_core::Result<()> {
    let data = native::core_network_filters();
    for item in native::network_filter_iterator(data) {
        let filter = HSTRING::from(item);
        webview.AddWebResourceRequestedFilter(&filter, COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL)?;
    }
    native::core_network_filters_delete(data);
    Ok(())
}

#[inline]
pub unsafe fn override_network(
    webview_request: &ICoreWebView2WebResourceRequest,
    uri: &String
) -> Option<Box<dyn NetworkResponse>> {
    let request_result = request::build(webview_request, uri);
    match request_result {
        Ok(request) => {
            let res = native::core_network_override(request.to_native());
            if !res.is_null() {
                match response::build(res) {
                    Ok(result) => {
                        return Some(result);
                    }
                    Err(error) => {
                        println!("[override_network][response] Uri: {}, Error: {:?}", uri, error);
                    }
                }
            }
        }
        Err(error) => {
            println!("[override_network][request] Uri: {}, Error: {:?}", uri, error);
        }
    }

    None
}

#[inline]
pub unsafe fn setup(mapgenie_resources_path: &str) {
    let uri_cstring_result = native::string_to_cstring(mapgenie_resources_path);
    match uri_cstring_result {
        Ok(uri_cstring) => {
            let config = native::AppConfig {
                mapgenie_resources_path: uri_cstring.as_ptr(),
            };

            native::core_app_config(config);
        }
        Err(error) => {
            println!("[setup] Error: {:?}", error);
        }
    }
}