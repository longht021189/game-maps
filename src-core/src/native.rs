#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::ffi::{CStr, CString};
use std::str::Utf8Error;
use crate::types;

include!("./gen/bindings.rs");

pub struct NetworkFilterIterator {
    current: *mut NetworkFilter,
}

impl Iterator for NetworkFilterIterator {
    type Item = &'static str;

    fn next(&mut self) -> Option<Self::Item> {
        if self.current.is_null() {
            None
        } else {
            let current = self.current;

            unsafe {
                let data_ptr = (*current).value;
                self.current = (*current).next;
                if data_ptr.is_null() {
                    Some("")
                } else {
                    // Ignore invalid UTF-8 for simplicity
                    CStr::from_ptr(data_ptr).to_str().ok()
                }
            }
        }
    }
}

pub fn network_filter_iterator(head: *mut NetworkFilter) -> NetworkFilterIterator {
    NetworkFilterIterator { current: head }
}

pub fn string_to_cstring(s: &str) -> types::Result<CString> {
    CString::new(s).map_err(|e| types::Error::NativeError(types::NativeErrorInfo{
        line: line!(),
        column: column!(),
        file: file!().to_string(),
        message: format!("Convert String to CString Error: {:?}", e)
    }))
}

pub fn vec_to_cstring(s: Vec<u8>) -> types::Result<CString> {
    CString::new(s).map_err(|e| types::Error::NativeError(types::NativeErrorInfo{
        line: line!(),
        column: column!(),
        file: file!().to_string(),
        message: format!("Convert Vec<u8> to CString Error: {:?}", e)
    }))
}

pub fn cstring_to_string(s: *mut ::std::os::raw::c_char) -> types::Result<String> {
    unsafe {
        let c_str = CStr::from_ptr(s);

        match c_str.to_str() {
            Ok(data) => Ok(data.to_string()),
            Err(error) => {
                Err(types::Error::NativeError(types::NativeErrorInfo{
                    line: line!(),
                    column: column!(),
                    file: file!().to_string(),
                    message: format!("Convert CString to String Error: {:?}", error)
                }))
            }
        }
    }
}