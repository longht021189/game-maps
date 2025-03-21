use std::ffi::CStr;
use crate::{native, types};

pub struct WebResourceResponse {
    response: *mut native::NetworkResponse,
    content: String,
    content_type: String,
}

impl Drop for WebResourceResponse {
    fn drop(&mut self) {
        unsafe {
            native::core_network_response_delete(self.response);
        }
    }
}

impl types::NetworkResponse for WebResourceResponse {
    fn get_status(&self) -> types::Status {
        unsafe {
            match (*self.response).status {
                native::NetworkResponseStatus_NETWORK_RESPONSE_STATUS_OK => types::Status::Code_200_Success,
                native::NetworkResponseStatus_NETWORK_RESPONSE_STATUS_NO_CONTENT => types::Status::Code_204_NoContent,
                native::NetworkResponseStatus_NETWORK_RESPONSE_STATUS_NOT_FOUND => types::Status::Code_404_NotFound,
                native::NetworkResponseStatus_NETWORK_RESPONSE_STATUS_METHOD_NOT_ALLOWED => types::Status::Code_405_MethodNotAllowed,
                _ => types::Status::Code_404_NotFound,
            }
        }
    }

    fn get_content(&self) -> String {
        self.content.clone()
    }

    fn get_content_type(&self) -> String {
        self.content_type.clone()
    }
}

unsafe fn get_content(response: *mut native::NetworkResponse) -> String {
    if (*response).content.is_null() {
        "".to_string()
    } else {
        let c_str = CStr::from_ptr((*response).content);
        c_str.to_str().unwrap().to_string()
    }
}

unsafe fn get_content_type(response: *mut native::NetworkResponse) -> types::Result<String> {
    if (*response).headers.is_null() {
        Ok("".to_string())
    } else {
        let headers = native::cstring_to_string((*response).headers)?.to_lowercase();
        let content_type = headers
            .split("\n")
            .find(|line| line.contains("content-type:"))
            .map(|line| line.replace("content-type:", ""))
            .unwrap_or("".to_string());

        Ok(content_type.trim().to_string())
    }
}

pub unsafe fn build(
    response: *mut native::NetworkResponse
) -> types::Result<Box<dyn types::NetworkResponse>> {
    let content = get_content(response);
    let content_type = get_content_type(response)?;
    Ok(Box::new(WebResourceResponse { response, content, content_type }))
}