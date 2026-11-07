//! Serves every `http(s)` request of the WebView from Rust (reqwest) instead of
//! the WebView's own network stack. Pages keep their real origin.
//!
//! The wry fork treats a custom protocol named after an origin, here the
//! wildcards `wry::ANY_HTTP_ORIGIN` / `wry::ANY_HTTPS_ORIGIN`, as an HTTP
//! interceptor (`WebViewBuilder::with_http_interceptor`). That is how this is
//! registered through Tauri's `register_asynchronous_uri_scheme_protocol`.
//! Tauri's own protocols (`ipc`, `tauri`, …) still take precedence.
//!
//! Ad and tracking requests (see [`blocklist`]) never leave the app: they are
//! answered with an empty `204 No Content`.
//!
//! Marking a mapgenie location as found / not found, and notes, are stored
//! locally instead of reaching mapgenie, see [`user_locations`] and [`user_notes`].

mod blocklist;
mod client;
mod headers;
mod user_locations;
mod user_notes;

use std::sync::Arc;

use tauri::http::{Request, Response, StatusCode, header};
use tauri::{Manager, Runtime, UriSchemeContext, UriSchemeResponder};

use crate::locations::LocationStore;

/// Handler for `register_asynchronous_uri_scheme_protocol`.
pub fn protocol<R: Runtime>(
    ctx: UriSchemeContext<'_, R>,
    request: Request<Vec<u8>>,
    responder: UriSchemeResponder,
) {
    let store = ctx
        .app_handle()
        .try_state::<Arc<LocationStore>>()
        .map(|store| store.inner().clone());
    tauri::async_runtime::spawn(async move {
        responder.respond(handle(request, store).await);
    });
}

async fn handle(request: Request<Vec<u8>>, store: Option<Arc<LocationStore>>) -> Response<Vec<u8>> {
    if let Some((action, location)) = user_locations::matches(request.method(), request.uri()) {
        return user_locations::handle(store, action, location, request.headers()).await;
    }
    if user_locations::matches_map_data(request.method(), request.uri()) {
        return user_locations::handle_map_data(store, request.headers()).await;
    }
    if let Some(page_path) = user_locations::matches_list(request.method(), request.uri()) {
        return user_locations::handle_list(store, page_path).await;
    }
    if let Some(call) = user_notes::matches(request.method(), request.uri()) {
        let (parts, body) = request.into_parts();
        return user_notes::handle(store, call, &parts.headers, body).await;
    }

    if blocklist::is_blocked(request.uri()) {
        return Response::builder()
            .status(StatusCode::NO_CONTENT)
            .header(headers::MARKER_NAME, headers::MARKER_VALUE)
            .header(headers::BLOCKED_NAME, headers::BLOCKED_VALUE)
            .body(Vec::new())
            .unwrap();
    }

    match forward(request).await {
        Ok(response) => response,
        Err(err) => Response::builder()
            .status(StatusCode::BAD_GATEWAY)
            .header(header::CONTENT_TYPE, "text/plain; charset=utf-8")
            .header(headers::MARKER_NAME, headers::MARKER_VALUE)
            .body(format!("proxy error: {err}").into_bytes())
            .unwrap(),
    }
}

async fn forward(request: Request<Vec<u8>>) -> Result<Response<Vec<u8>>, reqwest::Error> {
    let (mut parts, body) = request.into_parts();
    headers::prepare_request(&mut parts.headers);

    let upstream = client::get()
        .request(parts.method, parts.uri.to_string())
        .headers(parts.headers)
        .body(body)
        .send()
        .await?;

    let status = upstream.status();
    let mut response_headers = upstream.headers().clone();
    let body = upstream.bytes().await?;
    headers::prepare_response(&mut response_headers);

    let mut response = Response::new(body.to_vec());
    *response.status_mut() = status;
    *response.headers_mut() = response_headers;
    Ok(response)
}
