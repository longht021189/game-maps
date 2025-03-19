use webview2_com::Microsoft::Web::WebView2::Win32::{
    ICoreWebView2,
    ICoreWebView2WebResourceRequest,
    COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL
};
use windows::core::HSTRING;

pub enum Status {
    Code_200_Success,
    Code_204_NoContent,
    Code_404_NotFound,
    Code_405_MethodNotAllowed,
}

pub trait NetworkResponse {
    fn get_status(&self) -> Status;
    fn get_content(&self) -> String;
    fn get_content_type(&self) -> String;
}

#[inline]
pub fn is_network_override() -> bool {
    println!(">>>>>>>>>>>>>>>>>> is_network_override");
    true
}

#[inline]
pub unsafe fn add_web_resource_requested_filter(
    webview: &ICoreWebView2
) -> windows_core::Result<()> {
    println!(">>>>>>>>>>>>>>>>>> add_web_resource_requested_filter");
    let filter = HSTRING::from("https://*");
    webview.AddWebResourceRequestedFilter(&filter, COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL)?;
    Ok(())
}

#[inline]
pub unsafe fn override_network(
    webview_request: &ICoreWebView2WebResourceRequest,
    uri: &String
) -> Option<Box<dyn NetworkResponse>> {
    println!(">>>>>>>>>>>>>>>>>> uri: {}", uri);
    None
}
