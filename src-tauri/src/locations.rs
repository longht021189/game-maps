//! What the user saves on each mapgenie map (found locations, notes), kept locally in redb instead
//! of the mapgenie account (see `proxy::user_locations`, `proxy::user_notes`).
//!
//! One entry per map page: the key is the page's URL path
//! (`/phantom-blade-zero/maps/weeping-creek-valley`), the value is [`MapEntry`] as JSON
//! (`{"mapId":123,"locations":[388128,…],"notes":[{…},…]}`).

use std::collections::BTreeSet;
use std::error::Error;
use std::path::Path;

use redb::{Database, ReadableDatabase, ReadableTable, TableDefinition, TableError};
use serde::{Deserialize, Serialize};

const MAPS: TableDefinition<&str, &str> = TableDefinition::new("maps");

pub type StoreResult<T> = Result<T, Box<dyn Error + Send + Sync>>;

/// A note as mapgenie returns it (`id`, `map_id`, `title`, `latitude`, …).
pub type Note = serde_json::Map<String, serde_json::Value>;

#[derive(Debug, Default, PartialEq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct MapEntry {
    /// mapgenie's id of the map (`window.mapData.map.id`), as last sent by the page.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub map_id: Option<u64>,
    /// Ids of the locations marked as found, sorted.
    #[serde(default)]
    pub locations: BTreeSet<u64>,
    /// Notes, in creation order.
    #[serde(default)]
    pub notes: Vec<Note>,
}

pub struct LocationStore {
    db: Database,
}

impl LocationStore {
    /// Opens the database at `path`, creating it if needed.
    pub fn open(path: &Path) -> StoreResult<Self> {
        Ok(Self {
            db: Database::create(path)?,
        })
    }

    /// Marks `location` as found on the map page at `page_path`.
    pub fn add(&self, page_path: &str, map_id: u64, location: u64) -> StoreResult<()> {
        self.update(page_path, |entry| {
            entry.map_id = Some(map_id);
            entry.locations.insert(location);
        })
    }

    /// Marks `location` as not found on the map page at `page_path`.
    pub fn remove(&self, page_path: &str, map_id: u64, location: u64) -> StoreResult<()> {
        self.update(page_path, |entry| {
            entry.map_id = Some(map_id);
            entry.locations.remove(&location);
        })
    }

    /// The entry of the map page at `page_path`, if any.
    pub fn get(&self, page_path: &str) -> StoreResult<Option<MapEntry>> {
        let txn = self.db.begin_read()?;
        let table = match txn.open_table(MAPS) {
            Ok(table) => table,
            Err(TableError::TableDoesNotExist(_)) => return Ok(None),
            Err(err) => return Err(err.into()),
        };
        let Some(json) = table.get(page_path)? else {
            return Ok(None);
        };
        Ok(Some(serde_json::from_str(json.value())?))
    }

    /// Read-modify-write of the entry of `page_path` (empty if there is none yet), in a single
    /// write transaction.
    pub fn update<R>(
        &self,
        page_path: &str,
        change: impl FnOnce(&mut MapEntry) -> R,
    ) -> StoreResult<R> {
        let txn = self.db.begin_write()?;
        let result;
        {
            let mut table = txn.open_table(MAPS)?;
            let mut entry: MapEntry = match table.get(page_path)? {
                Some(json) => serde_json::from_str(json.value())?,
                None => MapEntry::default(),
            };
            result = change(&mut entry);
            table.insert(page_path, serde_json::to_string(&entry)?.as_str())?;
        }
        txn.commit()?;
        Ok(result)
    }
}

#[cfg(test)]
impl LocationStore {
    pub fn in_memory() -> Self {
        Self {
            db: Database::builder()
                .create_with_backend(redb::backends::InMemoryBackend::new())
                .unwrap(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const PAGE: &str = "/phantom-blade-zero/maps/weeping-creek-valley";

    #[test]
    fn adds_and_removes_locations_per_page() {
        let store = LocationStore::in_memory();
        assert_eq!(store.get(PAGE).unwrap(), None);

        store.add(PAGE, 7, 388128).unwrap();
        store.add(PAGE, 7, 5).unwrap();
        store.add(PAGE, 7, 388128).unwrap();
        store.add("/other/maps/x", 8, 1).unwrap();
        store.remove(PAGE, 7, 5).unwrap();
        store.remove(PAGE, 7, 999).unwrap();

        assert_eq!(
            store.get(PAGE).unwrap(),
            Some(MapEntry {
                map_id: Some(7),
                locations: BTreeSet::from([388128]),
                notes: Vec::new(),
            })
        );
        assert_eq!(
            store.get("/other/maps/x").unwrap().unwrap().locations,
            BTreeSet::from([1])
        );
    }

    #[test]
    fn stores_json_with_locations_and_notes() {
        let mut note = Note::new();
        note.insert("id".into(), "abc".into());
        let entry = MapEntry {
            map_id: Some(7),
            locations: BTreeSet::from([3, 1]),
            notes: vec![note],
        };
        assert_eq!(
            serde_json::to_string(&entry).unwrap(),
            r#"{"mapId":7,"locations":[1,3],"notes":[{"id":"abc"}]}"#
        );
    }

    #[test]
    fn reads_entries_saved_before_notes() {
        let entry: MapEntry =
            serde_json::from_str(r#"{"mapId":1092,"locations":[1063684]}"#).unwrap();
        assert_eq!(entry.map_id, Some(1092));
        assert_eq!(entry.locations, BTreeSet::from([1063684]));
        assert!(entry.notes.is_empty());
    }
}
