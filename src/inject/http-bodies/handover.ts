import {
  type HttpBodies,
  httpUrl,
  isLocalHost,
  newId,
  tag,
  toBase64,
} from './helpers';

declare global {
  interface Window {
    /** wry's native side, only on Android when HTTP interceptors are registered. */
    __wryHttpBodies?: HttpBodies;
    /** Set once `src/entrypoint/http-bodies.ts` has overridden the globals of this window. */
    __gameMapsOverridesInstalled?: boolean;
  }
}

/** Hands request bodies to wry on Android, see `src/entrypoint/http-bodies.ts`. */
export interface Handover {
  readonly win: Window;
  /**
   * `url` resolved against the document, if its body should be handed over: `http(s)` and not a
   * local host (custom schemes and `*.localhost` are the app's own, e.g. Tauri's IPC).
   */
  resolve(url: string | URL): URL | null;
  /** Hands `bytes` to wry and returns `url` tagged with their id. */
  handOver(url: URL, bytes: Uint8Array): string;
  /** Hands `bytes` to wry under an id chosen earlier (e.g. in `XMLHttpRequest.open`). */
  put(id: string, bytes: Uint8Array): void;
}

export function createHandover(win: Window, bodies: HttpBodies): Handover {
  const put = (id: string, bytes: Uint8Array) =>
    bodies.put(id, toBase64(bytes));
  return {
    win,
    resolve(url) {
      const resolved = httpUrl(url, win.document.baseURI);
      return resolved && !isLocalHost(resolved.hostname) ? resolved : null;
    },
    handOver(url, bytes) {
      const id = newId();
      put(id, bytes);
      return tag(url, id);
    },
    put,
  };
}
