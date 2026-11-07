//! Shared HTTP client used to reach the upstream site.

use std::sync::LazyLock;

static CLIENT: LazyLock<reqwest::Client> = LazyLock::new(|| {
    let builder = reqwest::Client::builder()
        // Cookies live in Rust: intercepted responses don't reliably feed the
        // WebView's cookie jar on every platform, so the session is kept here.
        .cookie_store(true)
        .redirect(redirect_policy());
    #[cfg(target_os = "android")]
    let builder = builder.tls_certs_only(root_certificates());
    builder.build().expect("failed to build HTTP client")
});

/// Redirects are handed back to the WebView, which follows them so its URL stays
/// in sync with the page it shows. wry cannot deliver `3xx` responses on
/// Android, so there they are followed here instead (the page keeps the
/// original URL).
fn redirect_policy() -> reqwest::redirect::Policy {
    if cfg!(target_os = "android") {
        reqwest::redirect::Policy::default()
    } else {
        reqwest::redirect::Policy::none()
    }
}

/// Trusted roots on Android: Mozilla's bundle (`webpki-root-certs`).
///
/// reqwest otherwise verifies with `rustls-platform-verifier`, which on Android must be
/// initialized through JNI (and ship its Kotlin component); without that every HTTPS request
/// panics. User-installed CAs are therefore not trusted on Android.
#[cfg(target_os = "android")]
fn root_certificates() -> Vec<reqwest::Certificate> {
    webpki_root_certs::TLS_SERVER_ROOT_CERTS
        .iter()
        .filter_map(|cert| reqwest::Certificate::from_der(cert).ok())
        .collect()
}

pub fn get() -> &'static reqwest::Client {
    &CLIENT
}
