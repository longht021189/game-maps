import { type ExtraHeaders, httpUrl } from '../http-bodies/helpers';

/**
 * mapgenie marks a location as found / not found with `PUT` / `DELETE
 * https://mapgenie.io/api/v1/user/locations/<id>`, and reads them with `GET
 * https://mapgenie.io/api/v1/user/map-data/<map id>`; notes are created / updated / deleted with
 * `POST https://mapgenie.io/api/v1/user/notes`, `PUT` / `DELETE .../notes/<id>`. The app answers
 * those in Rust from what it stores per map page (`src-tauri/src/proxy/user_locations.rs`,
 * `user_notes.rs`) instead of sending them to mapgenie; these headers tell it which map the request
 * comes from.
 */
export const MAP_ID_HEADER = 'X-Game-Maps-Map-Id';
export const PAGE_PATH_HEADER = 'X-Game-Maps-Page-Path';

const LOCATION_PATH = /^\/api\/v1\/user\/locations\/\d+\/?$/;
const MAP_DATA_PATH = /^\/api\/v1\/user\/map-data\/\d+\/?$/;
const NOTES_PATH = /^\/api\/v1\/user\/notes\/?$/;
const NOTE_PATH = /^\/api\/v1\/user\/notes\/[\w-]+\/?$/;

const isMapgenieApi = (url: URL) =>
  url.hostname.toLowerCase() === 'mapgenie.io';

/** Whether `method` + `url` marks a location as found / not found. */
export function isUserLocationRequest(method: string, url: URL): boolean {
  const upper = method.toUpperCase();
  return (
    (upper === 'PUT' || upper === 'DELETE') &&
    isMapgenieApi(url) &&
    LOCATION_PATH.test(url.pathname)
  );
}

/** Whether `method` + `url` reads the user's data (found locations) of a map. */
export function isUserMapDataRequest(method: string, url: URL): boolean {
  return (
    method.toUpperCase() === 'GET' &&
    isMapgenieApi(url) &&
    MAP_DATA_PATH.test(url.pathname)
  );
}

/** Whether `method` + `url` creates, updates or deletes a note. */
export function isUserNoteRequest(method: string, url: URL): boolean {
  if (!isMapgenieApi(url)) return false;
  const upper = method.toUpperCase();
  return upper === 'POST'
    ? NOTES_PATH.test(url.pathname)
    : (upper === 'PUT' || upper === 'DELETE') && NOTE_PATH.test(url.pathname);
}

/** `window.mapData.map.id`, set by mapgenie on map pages. */
export function mapId(win: Window): string | null {
  const data = (win as unknown as { mapData?: { map?: { id?: unknown } } })
    .mapData;
  const id = data?.map?.id;
  return typeof id === 'number' || (typeof id === 'string' && id)
    ? String(id)
    : null;
}

/**
 * Headers for the requests of `win`:
 * - marking a location: the map id and the page's URL path
 *   (`/phantom-blade-zero/maps/weeping-creek-valley`);
 * - reading the map's user data, and note calls: the page's URL path.
 *
 * Other requests get none.
 */
export function userLocationHeaders(win: Window): ExtraHeaders {
  return (method, url) => {
    const resolved = httpUrl(url, win.document.baseURI);
    if (!resolved) return null;
    if (
      isUserMapDataRequest(method, resolved) ||
      isUserNoteRequest(method, resolved)
    ) {
      return { [PAGE_PATH_HEADER]: win.location.pathname };
    }
    if (!isUserLocationRequest(method, resolved)) return null;

    const headers: Record<string, string> = {
      [PAGE_PATH_HEADER]: win.location.pathname,
    };
    const id = mapId(win);
    if (id) headers[MAP_ID_HEADER] = id;
    return headers;
  };
}
