import type { Handover } from './handover';
import { type ExtraHeaders, hasBody, newId, syncBytes, tag } from './helpers';

/**
 * An `XMLHttpRequest` class extending `Base`. Requests get the headers `extraHeaders` asks for
 * (set in `open`). With `handover` (Android), a request with a body gets its URL tagged in `open`
 * and hands the body over in `send`; everything else behaves as `Base`.
 */
export function createXMLHttpRequest(
  Base: typeof XMLHttpRequest,
  handover: Handover | null,
  extraHeaders: ExtraHeaders | null = null,
): typeof XMLHttpRequest {
  return class XMLHttpRequest extends Base {
    #bodyId: string | null = null;

    open(
      method: string,
      url: string | URL,
      async?: boolean,
      username?: string | null,
      password?: string | null,
    ): void {
      this.#bodyId = null;
      const resolved =
        handover && hasBody(method) ? handover.resolve(url) : null;
      let target: string | URL = url;
      if (resolved) {
        this.#bodyId = newId();
        target = tag(resolved, this.#bodyId);
      }
      // `open(method, url)` is the same as `async = true`.
      super.open(method, target, async ?? true, username, password);

      const extra = extraHeaders?.(method, url);
      if (extra) {
        for (const [name, value] of Object.entries(extra)) {
          this.setRequestHeader(name, value);
        }
      }
    }

    send(body?: Document | XMLHttpRequestBodyInit | null): void {
      const id = this.#bodyId;
      if (!handover || !id || body == null) {
        super.send(body);
        return;
      }

      const bytes = syncBytes(body);
      if (bytes !== undefined) {
        handover.put(id, bytes);
        super.send(body);
        return;
      }

      // Blob / FormData: serialize first. The serialized `Content-Type` (multipart boundary)
      // must match the bytes, so those bytes are what gets sent.
      const response = new handover.win.Response(body as BodyInit);
      const contentType = response.headers.get('Content-Type');
      response.arrayBuffer().then(
        (buffer) => {
          const serialized = new Uint8Array(buffer);
          handover.put(id, serialized);
          if (contentType) this.setRequestHeader('Content-Type', contentType);
          super.send(serialized);
        },
        () => super.send(body),
      );
    }
  };
}
