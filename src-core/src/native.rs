#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::ffi::CStr;

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