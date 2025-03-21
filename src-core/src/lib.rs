use std::ffi::CString;
use std::fmt::Debug;
use std::result;
use webview2_com::Microsoft::Web::WebView2::Win32::{
    ICoreWebView2,
    ICoreWebView2WebResourceRequest,
    COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL
};
use webview2_com::take_pwstr;
use windows::core::HSTRING;
use windows_core::{BOOL, PWSTR};

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
    if let Ok(request) = request_result {
        let res = native::core_network_override(request.to_native());
        if !res.is_null() {
            return response::build(res);
        }
    }
    None
}

#[inline]
pub unsafe fn setup(mapgenie_resources_path: &str) {
    let uri_cstring = CString::new(mapgenie_resources_path)
        .expect("CString::new failed");

    let config = native::AppConfig {
        mapgenie_resources_path: uri_cstring.as_ptr(),
    };

    native::core_app_config(config);
}