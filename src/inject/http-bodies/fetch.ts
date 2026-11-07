import type { Handover } from './handover';
import { type ExtraHeaders, hasBody } from './helpers';

/**
 * A `fetch` calling `original`. Requests get the headers `extraHeaders` asks for. With `handover`
 * (Android), requests with a body hand it over and go to the tagged URL; everything else is passed
 * through unchanged.
 */
export function createFetch(
  original: typeof fetch,
  handover: Handover | null,
  extraHeaders: ExtraHeaders | null = null,
): typeof fetch {
  return async function fetch(
    this: unknown,
    input: RequestInfo | URL,
    init?: RequestInit,
  ) {
    if (extraHeaders) init = withExtraHeaders(input, init, extraHeaders);
    if (!handover) return original.call(this, input, init);

    const request = new handover.win.Request(input, init);
    const url = hasBody(request.method) ? handover.resolve(request.url) : null;
    if (!url) return original.call(this, input, init);

    const bytes = new Uint8Array(await request.clone().arrayBuffer());
    if (!bytes.length) return original.call(this, input, init);

    return original.call(
      this,
      new handover.win.Request(handover.handOver(url, bytes), {
        method: request.method,
        // Includes the `Content-Type` (and multipart boundary) matching `bytes`.
        headers: request.headers,
        body: bytes,
        mode: request.mode === 'navigate' ? 'same-origin' : request.mode,
        credentials: request.credentials,
        cache: request.cache,
        redirect: request.redirect,
        referrer: request.referrer,
        referrerPolicy: request.referrerPolicy,
        integrity: request.integrity,
        keepalive: request.keepalive,
        signal: request.signal,
      }),
    );
  };
}

/**
 * `init` with the headers `extraHeaders` asks for, added to the request's own. `input` is not
 * read, so a `Request` body stays unused.
 */
function withExtraHeaders(
  input: RequestInfo | URL,
  init: RequestInit | undefined,
  extraHeaders: ExtraHeaders,
): RequestInit | undefined {
  // Duck-typed: a `Request` may come from another frame.
  const request: Request | null =
    typeof input === 'object' && 'method' in input ? input : null;
  const url: string | URL = request ? request.url : (input as string | URL);
  const method = init?.method ?? request?.method ?? 'GET';
  const extra = extraHeaders(method, url);
  if (!extra) return init;

  // Like `fetch`, `init.headers` replaces the headers of a `Request` input.
  const headers = new Headers(init?.headers ?? request?.headers);
  for (const [name, value] of Object.entries(extra)) headers.set(name, value);
  return { ...init, headers };
}
