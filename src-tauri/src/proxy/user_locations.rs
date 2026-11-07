//! mapgenie marks a location as found / not found with
//! `PUT` / `DELETE https://mapgenie.io/api/v1/user/locations/<id>`. These never reach mapgenie:
//! they are stored locally ([`LocationStore`]) under the map page they were made from.
//!
//! The init script (`src/inject/mapgenie/locations.ts`) adds the map to the request:
//! - [`MAP_ID_HEADER`]: `window.mapData.map.id`;
//! - [`PAGE_PATH_HEADER`]: the page's URL path (`/phantom-blade-zero/maps/weeping-creek-valley`),
//!   the key in the store.
//!
//! Without them, the request is answered `400` (and still not sent to mapgenie).
//!
//! The page reads them back, answered
//! `{"locations":{"388101":true,"388162":true},"gameLocationsCount":2,"hasPro":true,"notes":[…]}`
//! (`notes`: see `user_notes`), with:
//! - `GET https://mapgenie.io/api/v1/user/map-data/<map id>` (mapgenie's own call): the init
//!   script adds [`PAGE_PATH_HEADER`], `400` without it;
//! - `GET https://mapgenie.io<page path>/locations` (not a mapgenie URL).

use std::sync::Arc;

use tauri::http::{HeaderMap, Method, Response, StatusCode, Uri, header};

use super::headers;
use crate::locations::LocationStore;

pub const MAP_ID_HEADER: &str = "x-game-maps-map-id";
pub const PAGE_PATH_HEADER: &str = "x-game-maps-page-path";

/// Added (with the marker) to responses answered here.
const LOCAL_NAME: &str = "x-game-maps-local";
const LOCAL_VALUE: &str = "user-locations";

const PATH_PREFIX: &str = "/api/v1/user/locations/";
const MAP_DATA_PREFIX: &str = "/api/v1/user/map-data/";

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Action {
    /// `PUT`: marked as found.
    Add,
    /// `DELETE`: marked as not found.
    Remove,
}

/// The action and location id, if `method` + `uri` is one of the calls handled here.
pub fn matches(method: &Method, uri: &Uri) -> Option<(Action, u64)> {
    let action = match *method {
        Method::PUT => Action::Add,
        Method::DELETE => Action::Remove,
        _ => return None,
    };
    if !is_mapgenie(uri) {
        return None;
    }
    let id = uri.path().strip_prefix(PATH_PREFIX)?;
    let id = id.strip_suffix('/').unwrap_or(id);
    if !is_id(id) {
        return None;
    }
    Some((action, id.parse().ok()?))
}

fn is_id(id: &str) -> bool {
    !id.is_empty() && id.bytes().all(|b| b.is_ascii_digit())
}

/// Whether `method` + `uri` is `GET https://mapgenie.io/api/v1/user/map-data/<map id>`.
pub fn matches_map_data(method: &Method, uri: &Uri) -> bool {
    *method == Method::GET
        && is_mapgenie(uri)
        && uri
            .path()
            .strip_prefix(MAP_DATA_PREFIX)
            .map(|id| id.strip_suffix('/').unwrap_or(id))
            .is_some_and(is_id)
}

/// Answers `map-data` with the locations stored for the page in [`PAGE_PATH_HEADER`].
pub async fn handle_map_data(
    store: Option<Arc<LocationStore>>,
    request_headers: &HeaderMap,
) -> Response<Vec<u8>> {
    match page_path_of(request_headers) {
        Ok(page_path) => handle_list(store, page_path).await,
        Err(message) => error(StatusCode::BAD_REQUEST, message),
    }
}

/// The page path, if `method` + `uri` reads the locations of a map page:
/// `GET https://mapgenie.io/<game>/maps/<map>/locations`.
pub fn matches_list(method: &Method, uri: &Uri) -> Option<String> {
    if *method != Method::GET || !is_mapgenie(uri) {
        return None;
    }
    let page_path = uri.path().strip_suffix("/locations")?;
    // Only map pages, so mapgenie's own URLs (`/api/v1/user/locations`, …) are left alone.
    match page_path.split('/').collect::<Vec<_>>()[..] {
        ["", game, "maps", map] if !game.is_empty() && !map.is_empty() => {
            Some(page_path.to_owned())
        }
        _ => None,
    }
}

/// Answers the locations stored for `page_path`, as
/// `{"locations":{"<id>":true,…},"gameLocationsCount":<number of ids>,"hasPro":true,"notes":[…]}`.
pub async fn handle_list(
    store: Option<Arc<LocationStore>>,
    page_path: String,
) -> Response<Vec<u8>> {
    let Some(store) = store else {
        return error(
            StatusCode::INTERNAL_SERVER_ERROR,
            "location store unavailable".into(),
        );
    };

    let entry = tauri::async_runtime::spawn_blocking(move || store.get(&page_path)).await;
    let entry = match entry {
        Ok(Ok(entry)) => entry.unwrap_or_default(),
        Ok(Err(err)) => {
            return error(
                StatusCode::INTERNAL_SERVER_ERROR,
                format!("store error: {err}"),
            );
        }
        Err(err) => {
            return error(
                StatusCode::INTERNAL_SERVER_ERROR,
                format!("store error: {err}"),
            );
        }
    };

    let count = entry.locations.len();
    let found: serde_json::Map<String, serde_json::Value> = entry
        .locations
        .into_iter()
        .map(|id| (id.to_string(), serde_json::Value::Bool(true)))
        .collect();
    let body = serde_json::json!({
        "locations": found,
        "gameLocationsCount": count,
        "hasPro": true,
        "notes": entry
            .notes
            .into_iter()
            .map(|mut note| {
                // Notes saved before `user_id` was fixed had `null`.
                note.insert("user_id".into(), super::user_notes::USER_ID.into());
                note
            })
            .collect::<Vec<_>>(),
    });
    local(StatusCode::OK)
        .header(header::CONTENT_TYPE, "application/json; charset=utf-8")
        .header(header::CACHE_CONTROL, "no-store")
        .body(serde_json::to_vec(&body).unwrap())
        .unwrap()
}

pub(super) fn is_mapgenie(uri: &Uri) -> bool {
    uri.host()
        .is_some_and(|host| host.eq_ignore_ascii_case("mapgenie.io"))
}

/// Stores the change and answers like mapgenie: `PUT` → `200 {}`, `DELETE` → `204`.
pub async fn handle(
    store: Option<Arc<LocationStore>>,
    action: Action,
    location: u64,
    request_headers: &HeaderMap,
) -> Response<Vec<u8>> {
    let Some(store) = store else {
        return error(
            StatusCode::INTERNAL_SERVER_ERROR,
            "location store unavailable".into(),
        );
    };
    let (map_id, page_path) = match map_of(request_headers) {
        Ok(map) => map,
        Err(message) => return error(StatusCode::BAD_REQUEST, message),
    };

    let stored = tauri::async_runtime::spawn_blocking(move || match action {
        Action::Add => store.add(&page_path, map_id, location),
        Action::Remove => store.remove(&page_path, map_id, location),
    })
    .await;

    match stored {
        Ok(Ok(())) => match action {
            Action::Add => local(StatusCode::OK)
                .header(header::CONTENT_TYPE, "application/json; charset=utf-8")
                .body(b"{}".to_vec())
                .unwrap(),
            Action::Remove => local(StatusCode::NO_CONTENT).body(Vec::new()).unwrap(),
        },
        Ok(Err(err)) => error(
            StatusCode::INTERNAL_SERVER_ERROR,
            format!("store error: {err}"),
        ),
        Err(err) => error(
            StatusCode::INTERNAL_SERVER_ERROR,
            format!("store error: {err}"),
        ),
    }
}

/// Map id and page path sent by the init script.
fn map_of(request_headers: &HeaderMap) -> Result<(u64, String), String> {
    let map_id = header_value(request_headers, MAP_ID_HEADER)?
        .parse()
        .map_err(|_| format!("invalid header {MAP_ID_HEADER}"))?;
    Ok((map_id, page_path_of(request_headers)?))
}

/// Page path sent by the init script.
pub(super) fn page_path_of(request_headers: &HeaderMap) -> Result<String, String> {
    let page_path = header_value(request_headers, PAGE_PATH_HEADER)?;
    if !page_path.starts_with('/') {
        return Err(format!("invalid header {PAGE_PATH_HEADER}"));
    }
    Ok(page_path.to_owned())
}

fn header_value<'a>(request_headers: &'a HeaderMap, name: &str) -> Result<&'a str, String> {
    request_headers
        .get(name)
        .and_then(|value| value.to_str().ok())
        .map(str::trim)
        .filter(|value| !value.is_empty())
        .ok_or_else(|| format!("missing header {name}"))
}

pub(super) fn local(status: StatusCode) -> tauri::http::response::Builder {
    Response::builder()
        .status(status)
        .header(headers::MARKER_NAME, headers::MARKER_VALUE)
        .header(LOCAL_NAME, LOCAL_VALUE)
}

pub(super) fn error(status: StatusCode, message: String) -> Response<Vec<u8>> {
    local(status)
        .header(header::CONTENT_TYPE, "text/plain; charset=utf-8")
        .body(message.into_bytes())
        .unwrap()
}

#[cfg(test)]
mod tests {
    use super::*;
    use tauri::http::HeaderValue;

    const PAGE: &str = "/phantom-blade-zero/maps/weeping-creek-valley";

    fn uri(s: &str) -> Uri {
        s.parse().unwrap()
    }

    #[test]
    fn matches_put_and_delete_of_a_location() {
        let url = uri("https://mapgenie.io/api/v1/user/locations/388128");
        assert_eq!(matches(&Method::PUT, &url), Some((Action::Add, 388128)));
        assert_eq!(
            matches(&Method::DELETE, &url),
            Some((Action::Remove, 388128))
        );
        assert_eq!(
            matches(
                &Method::PUT,
                &uri("https://mapgenie.io/api/v1/user/locations/1/?x=1")
            ),
            Some((Action::Add, 1))
        );

        assert_eq!(matches(&Method::GET, &url), None);
        assert_eq!(matches(&Method::POST, &url), None);
        for other in [
            "https://cdn.mapgenie.io/api/v1/user/locations/388128",
            "https://example.com/api/v1/user/locations/388128",
            "https://mapgenie.io/api/v1/user/locations/",
            "https://mapgenie.io/api/v1/user/locations/abc",
            "https://mapgenie.io/api/v1/user/locations/1/2",
            "https://mapgenie.io/api/v1/user/locations",
        ] {
            assert_eq!(matches(&Method::PUT, &uri(other)), None, "{other}");
        }
    }

    fn map_headers(map_id: &str, page_path: &str) -> HeaderMap {
        let mut headers = HeaderMap::new();
        headers.insert(MAP_ID_HEADER, HeaderValue::from_str(map_id).unwrap());
        headers.insert(PAGE_PATH_HEADER, HeaderValue::from_str(page_path).unwrap());
        headers
    }

    #[test]
    fn stores_and_answers_locally() {
        let store = Arc::new(LocationStore::in_memory());
        let headers = map_headers("42", PAGE);
        let run = |action, location| {
            tauri::async_runtime::block_on(handle(Some(store.clone()), action, location, &headers))
        };

        let put = run(Action::Add, 388128);
        assert_eq!(put.status(), StatusCode::OK);
        assert_eq!(put.body(), b"{}");
        run(Action::Add, 5);

        let delete = run(Action::Remove, 5);
        assert_eq!(delete.status(), StatusCode::NO_CONTENT);
        assert!(delete.body().is_empty());

        let entry = store.get(PAGE).unwrap().unwrap();
        assert_eq!(entry.map_id, Some(42));
        assert_eq!(entry.locations.into_iter().collect::<Vec<_>>(), [388128]);
    }

    #[test]
    fn matches_the_locations_of_a_map_page() {
        assert_eq!(
            matches_list(
                &Method::GET,
                &uri(&format!("https://mapgenie.io{PAGE}/locations?x=1"))
            ),
            Some(PAGE.to_owned())
        );
        assert_eq!(
            matches_list(
                &Method::PUT,
                &uri(&format!("https://mapgenie.io{PAGE}/locations"))
            ),
            None
        );
        for other in [
            "https://mapgenie.io/api/v1/user/locations",
            "https://mapgenie.io/game/maps/locations",
            "https://mapgenie.io/game/maps//locations",
            "https://mapgenie.io/game/x/map/locations",
            "https://mapgenie.io/game/maps/map/extra/locations",
            "https://mapgenie.io/game/maps/map/locations/1",
            "https://cdn.mapgenie.io/game/maps/map/locations",
        ] {
            assert_eq!(matches_list(&Method::GET, &uri(other)), None, "{other}");
        }
    }

    #[test]
    fn lists_the_stored_locations() {
        let store = Arc::new(LocationStore::in_memory());
        let list = |page: &str| {
            let response =
                tauri::async_runtime::block_on(handle_list(Some(store.clone()), page.to_owned()));
            assert_eq!(response.status(), StatusCode::OK);
            String::from_utf8(response.into_body()).unwrap()
        };

        let empty = r#"{"gameLocationsCount":0,"hasPro":true,"locations":{},"notes":[]}"#;
        assert_eq!(list(PAGE), empty);

        store.add(PAGE, 42, 388162).unwrap();
        store.add(PAGE, 42, 388101).unwrap();
        store.add("/other/maps/x", 7, 1).unwrap();
        assert_eq!(
            list(PAGE),
            r#"{"gameLocationsCount":2,"hasPro":true,"locations":{"388101":true,"388162":true},"notes":[]}"#
        );

        store.remove(PAGE, 42, 388101).unwrap();
        store.remove(PAGE, 42, 388162).unwrap();
        assert_eq!(list(PAGE), empty);
    }

    #[test]
    fn matches_map_data() {
        let url = uri("https://mapgenie.io/api/v1/user/map-data/1092");
        assert!(super::matches_map_data(&Method::GET, &url));
        assert!(super::matches_map_data(
            &Method::GET,
            &uri("https://mapgenie.io/api/v1/user/map-data/1092/?x=1")
        ));
        assert!(!super::matches_map_data(&Method::POST, &url));
        for other in [
            "https://cdn.mapgenie.io/api/v1/user/map-data/1092",
            "https://mapgenie.io/api/v1/user/map-data/",
            "https://mapgenie.io/api/v1/user/map-data/abc",
            "https://mapgenie.io/api/v1/user/map-data/1/2",
        ] {
            assert!(
                !super::matches_map_data(&Method::GET, &uri(other)),
                "{other}"
            );
        }
    }

    #[test]
    fn answers_map_data_with_the_page_locations() {
        let store = Arc::new(LocationStore::in_memory());
        store.add(PAGE, 1092, 388162).unwrap();
        store.add(PAGE, 1092, 388101).unwrap();

        let mut headers = HeaderMap::new();
        headers.insert(PAGE_PATH_HEADER, HeaderValue::from_static(PAGE));
        let response =
            tauri::async_runtime::block_on(handle_map_data(Some(store.clone()), &headers));
        assert_eq!(response.status(), StatusCode::OK);
        assert_eq!(
            response.body(),
            br#"{"gameLocationsCount":2,"hasPro":true,"locations":{"388101":true,"388162":true},"notes":[]}"#
        );

        let missing =
            tauri::async_runtime::block_on(handle_map_data(Some(store), &HeaderMap::new()));
        assert_eq!(missing.status(), StatusCode::BAD_REQUEST);
    }

    #[test]
    fn rejects_requests_without_the_map() {
        let store = Arc::new(LocationStore::in_memory());
        for headers in [
            HeaderMap::new(),
            map_headers("abc", PAGE),
            map_headers("42", "no-slash"),
        ] {
            let response = tauri::async_runtime::block_on(handle(
                Some(store.clone()),
                Action::Add,
                1,
                &headers,
            ));
            assert_eq!(response.status(), StatusCode::BAD_REQUEST);
        }
        assert_eq!(store.get(PAGE).unwrap(), None);
    }
}
