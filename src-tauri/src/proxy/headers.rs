//! Header cleanup between the WebView and upstream.

use tauri::http::{header, HeaderMap, HeaderName, HeaderValue};

/// Marks every response served by this proxy, so it shows up in the WebView's
/// devtools (Network tab) as having gone through the app.
pub const MARKER_NAME: &str = "x-game-maps-proxy";
pub const MARKER_VALUE: &str = concat!("game-maps-app/", env!("CARGO_PKG_VERSION"));

/// Added (with the marker) to responses of requests blocked as ads or tracking.
pub const BLOCKED_NAME: &str = "x-game-maps-blocked";
pub const BLOCKED_VALUE: &str = "ad";

/// Request headers the WebView sends that must not be forwarded as-is.
const DROP_REQUEST: [HeaderName; 5] = [
    header::HOST,
    header::CONTENT_LENGTH,
    header::CONNECTION,
    // reqwest negotiates and decodes compression itself.
    header::ACCEPT_ENCODING,
    // Handled by the Rust cookie store.
    header::COOKIE,
];

/// Response headers that no longer describe the (decoded) body we hand back,
/// or that are managed on the Rust side.
const DROP_RESPONSE: [HeaderName; 6] = [
    header::CONTENT_ENCODING,
    header::CONTENT_LENGTH,
    header::TRANSFER_ENCODING,
    header::CONNECTION,
    header::SET_COOKIE,
    // The app embeds sites (mapgenie) in an `<iframe>` of its own origin, see `frame_ancestors`.
    header::X_FRAME_OPTIONS,
];

/// WebView → upstream.
pub fn prepare_request(headers: &mut HeaderMap) {
    for name in &DROP_REQUEST {
        headers.remove(name);
    }
}

/// Upstream → WebView.
pub fn prepare_response(headers: &mut HeaderMap) {
    for name in &DROP_RESPONSE {
        headers.remove(name);
    }
    allow_framing(headers);
    headers.insert(
        HeaderName::from_static(MARKER_NAME),
        HeaderValue::from_static(MARKER_VALUE),
    );
}

/// Removes the CSP `frame-ancestors` directive, which (like `X-Frame-Options`) stops the
/// app's `<iframe>` from showing the page. Other directives are kept.
fn allow_framing(headers: &mut HeaderMap) {
    for name in [
        header::CONTENT_SECURITY_POLICY,
        header::CONTENT_SECURITY_POLICY_REPORT_ONLY,
    ] {
        let policies: Vec<HeaderValue> = headers
            .get_all(&name)
            .iter()
            .filter_map(|value| {
                let Ok(policy) = value.to_str() else {
                    return Some(value.clone());
                };
                let kept = without_frame_ancestors(policy);
                if kept.is_empty() {
                    return None;
                }
                HeaderValue::from_str(&kept).ok()
            })
            .collect();
        headers.remove(&name);
        for policy in policies {
            headers.append(&name, policy);
        }
    }
}

/// `policy` without its `frame-ancestors` directive.
fn without_frame_ancestors(policy: &str) -> String {
    policy
        .split(';')
        .map(str::trim)
        .filter(|directive| {
            !directive.is_empty()
                && !directive
                    .split_ascii_whitespace()
                    .next()
                    .is_some_and(|name| name.eq_ignore_ascii_case("frame-ancestors"))
        })
        .collect::<Vec<_>>()
        .join("; ")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn allows_framing() {
        let mut headers = HeaderMap::new();
        headers.insert(header::X_FRAME_OPTIONS, HeaderValue::from_static("SAMEORIGIN"));
        headers.insert(
            header::CONTENT_SECURITY_POLICY,
            HeaderValue::from_static("default-src 'self'; Frame-Ancestors 'self'; img-src *"),
        );
        headers.insert(
            header::CONTENT_SECURITY_POLICY_REPORT_ONLY,
            HeaderValue::from_static("frame-ancestors 'none'"),
        );

        prepare_response(&mut headers);

        assert!(headers.get(header::X_FRAME_OPTIONS).is_none());
        assert_eq!(
            headers.get(header::CONTENT_SECURITY_POLICY).unwrap(),
            "default-src 'self'; img-src *"
        );
        assert!(headers
            .get(header::CONTENT_SECURITY_POLICY_REPORT_ONLY)
            .is_none());
    }
}
