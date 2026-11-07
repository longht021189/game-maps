import { beforeEach, describe, expect, rs, test } from '@rstest/core';
import { createFetch } from '../src/inject/http-bodies/fetch';
import {
  createHandover,
  type Handover,
} from '../src/inject/http-bodies/handover';
import {
  BODY_ID_PARAM,
  formBytes,
  hasBody,
  httpUrl,
  isLocalHost,
  isRemoteDocument,
  syncBytes,
  tag,
  toBase64,
} from '../src/inject/http-bodies/helpers';
import { createXMLHttpRequest } from '../src/inject/http-bodies/xhr';

const decode = (bytes: Uint8Array | undefined) =>
  new TextDecoder().decode(bytes);

describe('helpers', () => {
  test('hasBody', () => {
    expect(hasBody('POST')).toBe(true);
    expect(hasBody('put')).toBe(true);
    expect(hasBody('GET')).toBe(false);
    expect(hasBody('head')).toBe(false);
    expect(hasBody(undefined)).toBe(false);
  });

  test('httpUrl resolves http(s) URLs only', () => {
    expect(httpUrl('/a?b=1', 'https://app.com/x/')?.href).toBe(
      'https://app.com/a?b=1',
    );
    expect(httpUrl('data:text/plain,hi', 'https://app.com/')).toBeNull();
    expect(httpUrl('blob:https://app.com/1', 'https://app.com/')).toBeNull();
  });

  test('tag keeps the existing query and hash byte for byte', () => {
    expect(tag(new URL('https://app.com/a'), 'k1')).toBe(
      `https://app.com/a?${BODY_ID_PARAM}=k1`,
    );
    expect(tag(new URL('https://app.com/a?x=a%20b&y#h'), 'k1')).toBe(
      `https://app.com/a?x=a%20b&y&${BODY_ID_PARAM}=k1#h`,
    );
  });

  test('toBase64 handles binary and large bodies', () => {
    expect(toBase64(new Uint8Array([0, 1, 2, 255]))).toBe('AAEC/w==');
    const large = new Uint8Array(100_000).fill(65);
    expect(atob(toBase64(large)).length).toBe(100_000);
  });

  test('syncBytes', () => {
    expect(decode(syncBytes('héllo'))).toBe('héllo');
    expect(decode(syncBytes(new URLSearchParams({ a: 'x y' })))).toBe('a=x+y');
    expect(syncBytes(new Uint8Array([1, 2, 3]).subarray(1))).toEqual(
      new Uint8Array([2, 3]),
    );
    expect(syncBytes(new Blob(['x']))).toBeUndefined();
    expect(syncBytes(new FormData())).toBeUndefined();
  });

  test('formBytes encodes url-encoded and plain-text forms, not multipart', () => {
    const form = document.createElement('form');
    form.innerHTML =
      '<input name="a" value="x y&amp;z"><input name="b" value="é">';
    expect(decode(formBytes(form, null))).toBe('a=x+y%26z&b=%C3%A9');

    form.enctype = 'text/plain';
    expect(decode(formBytes(form, null))).toBe('a=x y&z\r\nb=é\r\n');

    form.enctype = 'multipart/form-data';
    expect(formBytes(form, null)).toBeUndefined();
  });
});

describe('app document and local hosts are ignored', () => {
  test('isLocalHost', () => {
    expect(isLocalHost('localhost')).toBe(true);
    expect(isLocalHost('tauri.localhost')).toBe(true);
    expect(isLocalHost('IPC.LOCALHOST')).toBe(true);
    expect(isLocalHost('127.0.0.1')).toBe(true);
    expect(isLocalHost('[::1]')).toBe(true);
    expect(isLocalHost('mapgenie.io')).toBe(false);
    expect(isLocalHost('localhost.example.com')).toBe(false);
  });

  test('isRemoteDocument', () => {
    const at = (url: string) => isRemoteDocument(new URL(url));
    expect(at('https://mapgenie.io/')).toBe(true);
    expect(at('http://localhost:3000/')).toBe(false);
    expect(at('http://tauri.localhost/')).toBe(false);
    expect(at('tauri://localhost/')).toBe(false);
    expect(at('about:blank')).toBe(false);
  });
});

describe('overrides', () => {
  let stored: Map<string, string>;
  let handover: Handover;

  beforeEach(() => {
    stored = new Map();
    handover = createHandover(window, {
      put: (id, base64) => stored.set(id, base64),
    });
  });

  const storedText = (url: string) => {
    const id = new URL(url).searchParams.get(BODY_ID_PARAM);
    return id ? atob(stored.get(id) as string) : undefined;
  };

  describe('fetch', () => {
    let original: ReturnType<typeof rs.fn>;
    beforeEach(() => {
      original = rs.fn(async () => new Response('ok'));
    });

    test('without handover, passes everything through', async () => {
      const fetch = createFetch(
        original as unknown as typeof globalThis.fetch,
        null,
      );
      const init = { method: 'POST', body: 'hello' };
      await fetch('https://app.com/api', init);
      expect(original.mock.calls[0]).toEqual(['https://app.com/api', init]);
    });

    test('hands the body over and tags the URL', async () => {
      const fetch = createFetch(
        original as unknown as typeof globalThis.fetch,
        handover,
      );
      await fetch('https://app.com/api?x=1', { method: 'POST', body: 'hello' });

      const request = original.mock.calls[0][0] as Request;
      expect(new URL(request.url).searchParams.get('x')).toBe('1');
      expect(storedText(request.url)).toBe('hello');
      expect(await request.text()).toBe('hello');
    });

    test('leaves bodiless and local requests alone', async () => {
      const fetch = createFetch(
        original as unknown as typeof globalThis.fetch,
        handover,
      );
      await fetch('https://app.com/api');
      await fetch('https://app.com/api', { method: 'POST' });
      await fetch('http://ipc.localhost/cmd', { method: 'POST', body: '{}' });

      expect(original.mock.calls.map((call) => call[0])).toEqual([
        'https://app.com/api',
        'https://app.com/api',
        'http://ipc.localhost/cmd',
      ]);
      expect(stored.size).toBe(0);
    });
  });

  describe('XMLHttpRequest', () => {
    /** Records what reaches the real `open` / `send`. */
    class FakeXMLHttpRequest {
      static calls: unknown[][] = [];
      headers: Record<string, string> = {};
      open(...args: unknown[]) {
        FakeXMLHttpRequest.calls.push(['open', ...args]);
      }
      send(body?: unknown) {
        FakeXMLHttpRequest.calls.push(['send', body]);
      }
      setRequestHeader(name: string, value: string) {
        this.headers[name] = value;
      }
    }
    const Fake = FakeXMLHttpRequest as unknown as typeof XMLHttpRequest;

    beforeEach(() => {
      FakeXMLHttpRequest.calls = [];
    });

    test('without handover, behaves as the original', () => {
      const XHR = createXMLHttpRequest(Fake, null);
      const xhr = new XHR();
      xhr.open('POST', 'https://app.com/api');
      xhr.send('hello');
      expect(xhr).toBeInstanceOf(Fake);
      expect(FakeXMLHttpRequest.calls).toEqual([
        ['open', 'POST', 'https://app.com/api', true, undefined, undefined],
        ['send', 'hello'],
      ]);
    });

    test('tags the URL in open and hands the body over in send', () => {
      const XHR = createXMLHttpRequest(Fake, handover);
      const xhr = new XHR();
      xhr.open('PUT', 'https://app.com/api?x=1', false);
      xhr.send('{"a":1}');

      const [open, send] = FakeXMLHttpRequest.calls;
      const url = open[2] as string;
      expect(open[3]).toBe(false);
      expect(new URL(url).searchParams.get('x')).toBe('1');
      expect(storedText(url)).toBe('{"a":1}');
      expect(send).toEqual(['send', '{"a":1}']);
    });

    test('serializes Blob bodies before sending', async () => {
      const XHR = createXMLHttpRequest(Fake, handover);
      const xhr = new XHR();
      xhr.open('POST', 'https://app.com/upload');
      xhr.send(new Blob(['blob-data'], { type: 'text/plain' }));
      await new Promise((resolve) => setTimeout(resolve, 10));

      const [open, send] = FakeXMLHttpRequest.calls;
      expect(storedText(open[2] as string)).toBe('blob-data');
      expect(new TextDecoder().decode(send[1] as Uint8Array)).toBe('blob-data');
      expect(
        (xhr as unknown as FakeXMLHttpRequest).headers['Content-Type'],
      ).toBe('text/plain');
    });

    test('leaves GET and local requests alone', () => {
      const XHR = createXMLHttpRequest(Fake, handover);
      const get = new XHR();
      get.open('GET', 'https://app.com/api');
      const local = new XHR();
      local.open('POST', 'http://ipc.localhost/cmd');
      local.send('{}');

      expect(FakeXMLHttpRequest.calls.map((call) => call.slice(0, 3))).toEqual([
        ['open', 'GET', 'https://app.com/api'],
        ['open', 'POST', 'http://ipc.localhost/cmd'],
        ['send', '{}'],
      ]);
      expect(stored.size).toBe(0);
    });
  });
});
