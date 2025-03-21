use std::ffi::CString;
use std::fmt::Debug;
use webview2_com::Microsoft::Web::WebView2::Win32::ICoreWebView2WebResourceRequest;
use webview2_com::take_pwstr;
use windows::Win32::System::Com::IStream;
use windows_core::{Interface, BOOL, PWSTR};
use crate::{native, types};

pub struct WebResourceRequest {
    uri_cstring: CString,
    request: *mut native::NetworkRequest,
    headers: Vec<CString>,
    content_cstring: Option<CString>,
}

impl WebResourceRequest {
    pub unsafe fn to_native(&self) -> *mut native::NetworkRequest {
        self.request
    }
}

impl Drop for WebResourceRequest {
    fn drop(&mut self) {
        unsafe { native::core_network_request_delete(self.request); }
    }
}

unsafe fn get_method(
    webview_request: &ICoreWebView2WebResourceRequest
) -> types::Result<native::NetworkRequestMethod> {
    let mut method = PWSTR::null();

    webview_request.Method(&mut method)
        .map_err(|e| types::Error::NativeError(types::NativeErrorInfo{
            line: line!(),
            column: column!(),
            file: file!().to_string(),
            message: format!("Get Method Error: {:?}", e)
        }))?;

    let method = take_pwstr(method);

    match method.as_str() {
        "GET" => Ok(native::NetworkRequestMethod_NETWORK_REQUEST_METHOD_GET),
        "PUT" => Ok(native::NetworkRequestMethod_NETWORK_REQUEST_METHOD_PUT),
        "DELETE" => Ok(native::NetworkRequestMethod_NETWORK_REQUEST_METHOD_DELETE),
        "POST" => Ok(native::NetworkRequestMethod_NETWORK_REQUEST_METHOD_POST),
        _ => Err(types::Error::NativeError(types::NativeErrorInfo{
            line: line!(),
            column: column!(),
            file: file!().to_string(),
            message: format!("Method {} isn't supported", method)
        }))
    }
}

unsafe fn parse_headers(
    request: *mut native::NetworkRequest,
    webview_request: &ICoreWebView2WebResourceRequest,
    headers_data: &mut Vec<CString>
) -> types::Result<()> {
    let headers = webview_request.Headers()
        .map_err(|e| types::Error::NativeError(types::NativeErrorInfo{
            line: line!(),
            column: column!(),
            file: file!().to_string(),
            message: format!("Get Headers Error: {:?}", e)
        }))?
        .GetIterator()
        .map_err(|e| types::Error::NativeError(types::NativeErrorInfo{
            line: line!(),
            column: column!(),
            file: file!().to_string(),
            message: format!("Get Headers Iterator Error: {:?}", e)
        }))?;

    let mut has_current = BOOL::default();

    headers.HasCurrentHeader(&mut has_current)
        .map_err(|e| types::Error::NativeError(types::NativeErrorInfo{
            line: line!(),
            column: column!(),
            file: file!().to_string(),
            message: format!("Check Headers Exists Error: {:?}", e)
        }))?;

    while has_current.as_bool() {
        let mut key = PWSTR::null();
        let mut value = PWSTR::null();

        headers.GetCurrentHeader(&mut key, &mut value)
            .map_err(|e| types::Error::NativeError(types::NativeErrorInfo{
                line: line!(),
                column: column!(),
                file: file!().to_string(),
                message: format!("Get Header Item Error: {:?}", e)
            }))?;

        let (key, value) = (take_pwstr(key), take_pwstr(value));
        let header_value = format!("{}: {}", key, value);
        let header_cstring = native::string_to_cstring(header_value.as_str())?;
        native::core_network_request_add_header(request, header_cstring.as_ptr());
        headers_data.push(header_cstring);
        headers.MoveNext(&mut has_current)
            .map_err(|e| types::Error::NativeError(types::NativeErrorInfo{
                line: line!(),
                column: column!(),
                file: file!().to_string(),
                message: format!("Move Next Header Error: {:?}", e)
            }))?;
    }

    Ok(())
}

unsafe fn read_content(
    webview_request: &ICoreWebView2WebResourceRequest
) -> types::Result<Vec<u8>> {
    let mut body_sent = Vec::new();
    if let Ok(content) = webview_request.Content() {
        let mut buffer: [u8; 1024] = [0; 1024];
        loop {
            let mut cb_read = 0;
            let content: IStream = content.cast()
                .map_err(|e| types::Error::NativeError(types::NativeErrorInfo{
                    line: line!(),
                    column: column!(),
                    file: file!().to_string(),
                    message: format!("Cast Error: {:?}", e)
                }))?;

            content
                .Read(
                    buffer.as_mut_ptr() as *mut _,
                    buffer.len() as u32,
                    Some(&mut cb_read),
                )
                .ok()
                .map_err(|e| types::Error::NativeError(types::NativeErrorInfo{
                    line: line!(),
                    column: column!(),
                    file: file!().to_string(),
                    message: format!("Read Error: {:?}", e)
                }))?;

            if cb_read == 0 {
                break;
            }

            body_sent.extend_from_slice(&buffer[..(cb_read as usize)]);
        }
    }
    Ok(body_sent)
}

pub unsafe fn build(
    webview_request: &ICoreWebView2WebResourceRequest,
    uri: &String
) -> types::Result<WebResourceRequest> {
    let method = get_method(webview_request)?;
    let uri_cstring = native::string_to_cstring(uri.as_str())?;
    let request = native::core_network_request_new(uri_cstring.as_ptr(), method);
    let mut headers_data: Vec<CString> = Vec::new();
    parse_headers(request, webview_request, &mut headers_data)?;

    let mut content_cstring: Option<CString> = None;
    match method {
        native::NetworkRequestMethod_NETWORK_REQUEST_METHOD_POST |
        native::NetworkRequestMethod_NETWORK_REQUEST_METHOD_PUT => {
            let content = read_content(webview_request)?;
            let content_cstring_data = native::vec_to_cstring(content)?;
            native::core_network_request_add_content(request, content_cstring_data.as_ptr());
            content_cstring = Some(content_cstring_data);
        }

        _ => {}
    }

    Ok(WebResourceRequest{ uri_cstring, request, headers: headers_data, content_cstring })
}
