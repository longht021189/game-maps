/**
 * Initialization script (all frames, every platform) overriding the page's network globals.
 *
 * `fetch` and `XMLHttpRequest` are replaced on every platform in remote `http(s)` pages (not the
 * app's own page on a custom scheme or localhost), so requests made by the page can be handled
 * here. On Android, they also hand request bodies to wry:
 * `WebViewClient.shouldInterceptRequest` never exposes the body, so before a request with a body
 * leaves, it is given to wry's native `HttpBodies` (`window.__wryHttpBodies.put(id, base64)`, a
 * synchronous `@JavascriptInterface`) and the URL is tagged with `__wry_body=<id>`. wry takes the
 * body back by that id and strips the parameter before the proxy (`src-tauri/src/proxy`) sees the
 * request.
 *
 * On mapgenie.io pages, it also injects style overrides hiding the ad slots
 * (`src/inject/mapgenie/`) and, as an example, traps `window.abc` (`src/inject/globals/abc.ts`).
 *
 * Built on its own by Rsbuild (`pnpm build:inject`, environment `inject`) into
 * `dist-inject/http-bodies.js`, which `src-tauri/src/inject.rs` embeds and registers.
 */

import { patchSendBeacon } from '../inject/http-bodies/beacon';
import { createFetch } from '../inject/http-bodies/fetch';
import { patchForms } from '../inject/http-bodies/forms';
import { createHandover } from '../inject/http-bodies/handover';
import { isRemoteDocument } from '../inject/http-bodies/helpers';
import { createXMLHttpRequest } from '../inject/http-bodies/xhr';
import { userLocationHeaders } from '../inject/mapgenie/locations';
import { injectStyles, isMapgenie } from '../inject/mapgenie/styles';
import { trapMapGenie } from '../secrets/trapMapGenie';

// The app's own page (custom scheme, dev server on localhost) is left alone: only remote pages
// such as the mapgenie iframe get the overrides.
if (isRemoteDocument(window.location) && !window.__gameMapsOverridesInstalled) {
  window.__gameMapsOverridesInstalled = true;

  // Only Android has wry's native side: elsewhere the overrides pass requests through.
  const bodies = window.__wryHttpBodies;
  const handover = bodies ? createHandover(window, bodies) : null;
  // mapgenie: tell the app which map a found / not found location belongs to
  // (`src/inject/mapgenie/locations.ts`).
  const extraHeaders = isMapgenie(window.location.hostname)
    ? userLocationHeaders(window)
    : null;

  window.fetch = createFetch(window.fetch, handover, extraHeaders);
  window.XMLHttpRequest = createXMLHttpRequest(
    window.XMLHttpRequest,
    handover,
    extraHeaders,
  );

  // Requests not made through `fetch` / `XMLHttpRequest` that may carry a body.
  if (handover) {
    patchSendBeacon(handover);
    patchForms(handover);
  }
}

if (isMapgenie(window.location.hostname)) {
  // Hide mapgenie's ad slots and the room kept for them (`src/inject/mapgenie/styles.css`).
  injectStyles(document);
  // Example of trapping a global set by page scripts (`window.abc`, unused by mapgenie).
  trapMapGenie(window);
}
