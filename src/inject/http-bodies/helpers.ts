/**
 * Pure helpers of the Android request body script, see `./index.ts`.
 */

/** Query parameter wry reads the body id from, and strips before the interceptor sees the URL. */
export const BODY_ID_PARAM = '__wry_body';

/** Native side of the contract: `HttpBodies`, a synchronous `@JavascriptInterface` from wry. */
export interface HttpBodies {
  put(id: string, base64: string): void;
}

/**
 * Headers to add to a request of `method` to `url` (as given to `fetch` / `XMLHttpRequest.open`,
 * possibly relative), or `null` to leave it as it is.
 */
export type ExtraHeaders = (
  method: string,
  url: string | URL,
) => Record<string, string> | null;

/** Whether requests with `method` carry a body (and so need handing over). */
export function hasBody(method: string | undefined | null): boolean {
  const upper = (method || 'GET').toUpperCase();
  return upper !== 'GET' && upper !== 'HEAD';
}

/**
 * `url` resolved against `base`, if it is `http(s)`: every such request goes through the app's
 * interceptor (`wry::ANY_HTTP_ORIGIN` / `wry::ANY_HTTPS_ORIGIN`).
 */
export function httpUrl(url: string | URL, base: string): URL | null {
  let parsed: URL;
  try {
    parsed = new URL(url, base);
  } catch {
    return null;
  }
  return parsed.protocol === 'http:' || parsed.protocol === 'https:'
    ? parsed
    : null;
}

/**
 * Whether `hostname` is the local machine or a `*.localhost` name: the dev server, and the app's
 * own custom protocols on Windows / Android (`http(s)://tauri.localhost`, `http://ipc.localhost`).
 */
export function isLocalHost(hostname: string): boolean {
  const host = hostname.toLowerCase();
  return (
    host === 'localhost' ||
    host.endsWith('.localhost') ||
    host === '127.0.0.1' ||
    host === '[::1]'
  );
}

/**
 * Whether a document at `location` gets the overrides: `http(s)` pages only, not the app's own
 * page (custom scheme such as `tauri://localhost`, or a local host).
 */
export function isRemoteDocument(location: {
  protocol: string;
  hostname: string;
}): boolean {
  return (
    (location.protocol === 'http:' || location.protocol === 'https:') &&
    !isLocalHost(location.hostname)
  );
}

/** `url` with the body id appended, leaving the existing query exactly as it was. */
export function tag(url: URL, id: string): string {
  const separator = url.search ? '&' : '?';
  return `${url.origin}${url.pathname}${url.search}${separator}${BODY_ID_PARAM}=${id}${url.hash}`;
}

let counter = 0;

/** A body id, unique within the page. */
export function newId(): string {
  counter += 1;
  return `${Date.now().toString(36)}-${counter.toString(36)}-${Math.random().toString(36).slice(2)}`;
}

export function toBase64(bytes: Uint8Array): string {
  let binary = '';
  for (let i = 0; i < bytes.length; i += 0x8000) {
    binary += String.fromCharCode(...bytes.subarray(i, i + 0x8000));
  }
  return btoa(binary);
}

/**
 * Bytes of a body that can be read synchronously, or `undefined` for the others
 * (`Blob`, `FormData`, streams), which need serializing first.
 */
export function syncBytes(body: unknown): Uint8Array | undefined {
  if (body == null) return new Uint8Array(0);
  if (typeof body === 'string') return new TextEncoder().encode(body);
  if (body instanceof URLSearchParams)
    return new TextEncoder().encode(body.toString());
  if (body instanceof ArrayBuffer) return new Uint8Array(body);
  if (ArrayBuffer.isView(body)) {
    return new Uint8Array(body.buffer, body.byteOffset, body.byteLength);
  }
  return undefined;
}

/**
 * Body of a form submission, as the browser encodes it, or `undefined` for
 * `multipart/form-data` (its boundary is chosen by the browser).
 */
export function formBytes(
  form: HTMLFormElement,
  submitter: HTMLElement | null,
): Uint8Array | undefined {
  const enctype = (
    submitter?.getAttribute('formenctype') ||
    form.enctype ||
    ''
  ).toLowerCase();
  if (enctype === 'multipart/form-data') return undefined;

  const data = new FormData(form, submitter ?? undefined);
  if (enctype === 'text/plain') {
    let text = '';
    data.forEach((value, name) => {
      text += `${name}=${value}\r\n`;
    });
    return new TextEncoder().encode(text);
  }

  const params = new URLSearchParams();
  // Url-encoded forms submit a file input as its file name.
  data.forEach((value, name) => {
    params.append(name, typeof value === 'string' ? value : value.name);
  });
  return new TextEncoder().encode(params.toString());
}
