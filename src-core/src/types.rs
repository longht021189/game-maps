use std::fmt::Debug;

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

pub type Result<T> = std::result::Result<T, Box<dyn Debug>>;