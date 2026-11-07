//! mapgenie's note calls, answered from the local store ([`LocationStore`]) instead of mapgenie:
//!
//! - `POST https://mapgenie.io/api/v1/user/notes`: creates the note in the body, answered with it
//!   plus `id` (random), `user_id` ([`USER_ID`]) and `created_at`;
//! - `PUT https://mapgenie.io/api/v1/user/notes/<id>`: updates it with the body, answered with
//!   the whole note;
//! - `DELETE https://mapgenie.io/api/v1/user/notes/<id>`: deletes it, answered `204`.
//!
//! Notes are kept under the map page in
//! [`PAGE_PATH_HEADER`](super::user_locations::PAGE_PATH_HEADER) (added by the init script,
//! `src/inject/mapgenie/locations.ts`): `400` without it. They are listed back with the found
//! locations (`notes` in `map-data`, see `user_locations`).

use std::sync::Arc;
use std::time::{SystemTime, UNIX_EPOCH};

use serde_json::Value;
use tauri::http::{HeaderMap, Method, Response, StatusCode, Uri, header};

use super::user_locations::{error, is_mapgenie, local, page_path_of};
use crate::locations::{LocationStore, Note, StoreResult};

const NOTES_PATH: &str = "/api/v1/user/notes";

/// `user_id` of every note.
pub const USER_ID: u64 = 9967907;

/// Fields set by the app, never taken from a request body.
const SERVER_FIELDS: [&str; 3] = ["id", "user_id", "created_at"];

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum NoteCall {
    Create,
    Update(String),
    Delete(String),
}

/// The note call, if `method` + `uri` is one of those handled here.
pub fn matches(method: &Method, uri: &Uri) -> Option<NoteCall> {
    if !is_mapgenie(uri) {
        return None;
    }
    let rest = uri.path().strip_prefix(NOTES_PATH)?;
    let rest = rest.strip_suffix('/').unwrap_or(rest);
    if rest.is_empty() {
        return (*method == Method::POST).then_some(NoteCall::Create);
    }
    let id = rest.strip_prefix('/')?;
    if id.is_empty()
        || !id
            .bytes()
            .all(|b| b.is_ascii_alphanumeric() || b == b'-' || b == b'_')
    {
        return None;
    }
    match *method {
        Method::PUT => Some(NoteCall::Update(id.to_owned())),
        Method::DELETE => Some(NoteCall::Delete(id.to_owned())),
        _ => None,
    }
}

pub async fn handle(
    store: Option<Arc<LocationStore>>,
    call: NoteCall,
    request_headers: &HeaderMap,
    body: Vec<u8>,
) -> Response<Vec<u8>> {
    let Some(store) = store else {
        return error(
            StatusCode::INTERNAL_SERVER_ERROR,
            "location store unavailable".into(),
        );
    };
    let page_path = match page_path_of(request_headers) {
        Ok(page_path) => page_path,
        Err(message) => return error(StatusCode::BAD_REQUEST, message),
    };
    let fields = match &call {
        NoteCall::Delete(_) => Note::new(),
        _ => match serde_json::from_slice::<Value>(&body) {
            Ok(Value::Object(fields)) => fields,
            _ => return error(StatusCode::BAD_REQUEST, "body is not a JSON object".into()),
        },
    };

    let applied =
        tauri::async_runtime::spawn_blocking(move || apply(&store, &page_path, call, fields)).await;
    match applied {
        Ok(Ok(response)) => response,
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

fn apply(
    store: &LocationStore,
    page_path: &str,
    call: NoteCall,
    fields: Note,
) -> StoreResult<Response<Vec<u8>>> {
    match call {
        NoteCall::Create => {
            let note = new_note(fields, new_id(), now());
            store.update(page_path, |entry| {
                if let Some(map_id) = note.get("map_id").and_then(Value::as_u64) {
                    entry.map_id = Some(map_id);
                }
                entry.notes.push(note.clone());
            })?;
            Ok(json(&note))
        }
        NoteCall::Update(id) => {
            let updated = store.update(page_path, |entry| {
                let note = entry.notes.iter_mut().find(|note| has_id(note, &id))?;
                update_note(note, fields);
                Some(note.clone())
            })?;
            Ok(match updated {
                Some(note) => json(&note),
                None => error(StatusCode::NOT_FOUND, format!("note {id} not found")),
            })
        }
        NoteCall::Delete(id) => {
            store.update(page_path, |entry| {
                entry.notes.retain(|note| !has_id(note, &id));
            })?;
            Ok(local(StatusCode::NO_CONTENT).body(Vec::new()).unwrap())
        }
    }
}

/// `fields` of a `POST` as a new note.
fn new_note(fields: Note, id: String, created_at: String) -> Note {
    let mut note: Note = fields
        .into_iter()
        .filter(|(name, _)| !SERVER_FIELDS.contains(&name.as_str()))
        .collect();
    note.insert("user_id".into(), USER_ID.into());
    note.insert("id".into(), id.into());
    note.insert("created_at".into(), created_at.into());
    note
}

/// Applies the `fields` of a `PUT` to `note`; like mapgenie, the result has a `category`.
fn update_note(note: &mut Note, fields: Note) {
    for (name, value) in fields {
        if !SERVER_FIELDS.contains(&name.as_str()) {
            note.insert(name, value);
        }
    }
    note.insert("user_id".into(), USER_ID.into());
    note.entry("category").or_insert(Value::Null);
}

fn has_id(note: &Note, id: &str) -> bool {
    note.get("id").and_then(Value::as_str) == Some(id)
}

fn json(note: &Note) -> Response<Vec<u8>> {
    local(StatusCode::OK)
        .header(header::CONTENT_TYPE, "application/json; charset=utf-8")
        .body(serde_json::to_vec(note).unwrap())
        .unwrap()
}

/// A random id like mapgenie's (`H6ZdMwjEj2G`): 11 alphanumeric characters.
fn new_id() -> String {
    const ALPHABET: &[u8] = b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    let mut id = String::with_capacity(11);
    let mut bytes = [0u8; 32];
    while id.len() < 11 {
        getrandom::fill(&mut bytes).expect("system random number generator");
        // Rejection sampling keeps every character equally likely.
        for byte in bytes {
            if id.len() < 11 && usize::from(byte) < 248 {
                id.push(char::from(ALPHABET[usize::from(byte) % ALPHABET.len()]));
            }
        }
    }
    id
}

/// The current time like mapgenie's `created_at`: `2026-10-11T05:29:26.000000Z`.
fn now() -> String {
    let secs = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|elapsed| elapsed.as_secs())
        .unwrap_or_default();
    timestamp(secs)
}

fn timestamp(unix_secs: u64) -> String {
    let (days, secs) = (unix_secs / 86_400, unix_secs % 86_400);
    // Civil date from days since 1970-01-01 (Howard Hinnant's `civil_from_days`).
    let z = days as i64 + 719_468;
    let era = z.div_euclid(146_097);
    let doe = z.rem_euclid(146_097);
    let yoe = (doe - doe / 1_460 + doe / 36_524 - doe / 146_096) / 365;
    let doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    let mp = (5 * doy + 2) / 153;
    let day = doy - (153 * mp + 2) / 5 + 1;
    let month = if mp < 10 { mp + 3 } else { mp - 9 };
    let year = yoe + era * 400 + i64::from(month <= 2);
    format!(
        "{year:04}-{month:02}-{day:02}T{:02}:{:02}:{:02}.000000Z",
        secs / 3_600,
        secs / 60 % 60,
        secs % 60
    )
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::proxy::user_locations::PAGE_PATH_HEADER;
    use tauri::http::HeaderValue;

    const PAGE: &str = "/phantom-blade-zero/maps/weeping-creek-valley";

    fn uri(s: &str) -> Uri {
        s.parse().unwrap()
    }

    #[test]
    fn matches_note_calls() {
        let notes = uri("https://mapgenie.io/api/v1/user/notes");
        let note = uri("https://mapgenie.io/api/v1/user/notes/H6ZdMwjEj2G?map_id=645");
        assert_eq!(matches(&Method::POST, &notes), Some(NoteCall::Create));
        assert_eq!(
            matches(&Method::PUT, &note),
            Some(NoteCall::Update("H6ZdMwjEj2G".into()))
        );
        assert_eq!(
            matches(&Method::DELETE, &note),
            Some(NoteCall::Delete("H6ZdMwjEj2G".into()))
        );

        assert_eq!(matches(&Method::GET, &notes), None);
        assert_eq!(matches(&Method::PUT, &notes), None);
        assert_eq!(matches(&Method::POST, &note), None);
        assert_eq!(matches(&Method::GET, &note), None);
        for other in [
            "https://cdn.mapgenie.io/api/v1/user/notes",
            "https://mapgenie.io/api/v1/user/notesx",
            "https://mapgenie.io/api/v1/user/notes/a/b",
        ] {
            assert_eq!(matches(&Method::POST, &uri(other)), None, "{other}");
            assert_eq!(matches(&Method::PUT, &uri(other)), None, "{other}");
        }
    }

    #[test]
    fn formats_timestamps_like_mapgenie() {
        assert_eq!(timestamp(0), "1970-01-01T00:00:00.000000Z");
        assert_eq!(timestamp(1_791_696_566), "2026-10-11T05:29:26.000000Z");
        assert_eq!(timestamp(951_782_400), "2000-02-29T00:00:00.000000Z");
    }

    #[test]
    fn ids_are_random_and_alphanumeric() {
        let (a, b) = (new_id(), new_id());
        assert_eq!(a.len(), 11);
        assert!(a.bytes().all(|b| b.is_ascii_alphanumeric()));
        assert_ne!(a, b);
    }

    fn object(json: &str) -> Note {
        serde_json::from_str(json).unwrap()
    }

    #[test]
    fn creates_and_updates_notes_like_mapgenie() {
        let created = new_note(
            object(r#"{"map_id":645,"profile_id":null,"title":"t","id":"forged"}"#),
            "H6ZdMwjEj2G".into(),
            "2026-10-11T05:29:26.000000Z".into(),
        );
        assert_eq!(
            Value::Object(created.clone()),
            serde_json::json!({
                "map_id": 645, "profile_id": null, "title": "t", "user_id": 9967907,
                "id": "H6ZdMwjEj2G", "created_at": "2026-10-11T05:29:26.000000Z",
            })
        );

        let mut note = created;
        update_note(
            &mut note,
            object(r#"{"title":"u","latitude":0.5,"id":"x"}"#),
        );
        assert_eq!(note["title"], "u");
        assert_eq!(note["latitude"], 0.5);
        assert_eq!(note["id"], "H6ZdMwjEj2G");
        assert_eq!(note["profile_id"], Value::Null);
        assert_eq!(note["category"], Value::Null);
        assert_eq!(note["user_id"], 9967907);
    }

    #[test]
    fn creates_updates_and_deletes_in_the_store() {
        let store = Arc::new(LocationStore::in_memory());
        let mut headers = HeaderMap::new();
        headers.insert(PAGE_PATH_HEADER, HeaderValue::from_static(PAGE));
        let run = |call, body: &str| {
            tauri::async_runtime::block_on(handle(
                Some(store.clone()),
                call,
                &headers,
                body.as_bytes().to_vec(),
            ))
        };
        let body_of =
            |response: Response<Vec<u8>>| serde_json::from_slice::<Value>(response.body()).unwrap();

        let created = run(NoteCall::Create, r#"{"map_id":645,"title":"a"}"#);
        assert_eq!(created.status(), StatusCode::OK);
        let id = body_of(created)["id"].as_str().unwrap().to_owned();

        let updated = run(
            NoteCall::Update(id.clone()),
            r#"{"map_id":645,"title":"b"}"#,
        );
        assert_eq!(updated.status(), StatusCode::OK);
        assert_eq!(body_of(updated)["title"], "b");

        let entry = store.get(PAGE).unwrap().unwrap();
        assert_eq!(entry.map_id, Some(645));
        assert_eq!(entry.notes.len(), 1);
        assert_eq!(entry.notes[0]["title"], "b");

        let missing = run(NoteCall::Update("nope".into()), "{}");
        assert_eq!(missing.status(), StatusCode::NOT_FOUND);
        assert_eq!(
            run(NoteCall::Create, "[]").status(),
            StatusCode::BAD_REQUEST
        );

        let deleted = run(NoteCall::Delete(id), "");
        assert_eq!(deleted.status(), StatusCode::NO_CONTENT);
        assert!(store.get(PAGE).unwrap().unwrap().notes.is_empty());

        let no_page = tauri::async_runtime::block_on(handle(
            Some(store.clone()),
            NoteCall::Create,
            &HeaderMap::new(),
            b"{}".to_vec(),
        ));
        assert_eq!(no_page.status(), StatusCode::BAD_REQUEST);
    }
}
