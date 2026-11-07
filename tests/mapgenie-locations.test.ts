import { beforeEach, describe, expect, rs, test } from '@rstest/core';
import { createFetch } from '../src/inject/http-bodies/fetch';
import { createXMLHttpRequest } from '../src/inject/http-bodies/xhr';
import {
  isUserLocationRequest,
  MAP_ID_HEADER,
  mapId,
  PAGE_PATH_HEADER,
  userLocationHeaders,
} from '../src/inject/mapgenie/locations';

const PAGE = '/phantom-blade-zero/maps/weeping-creek-valley';
const API = 'https://mapgenie.io/api/v1/user/locations/388128';

/** A mapgenie map page window, with only what the headers need. */
const mapPage = (mapData?: unknown) =>
  ({
    document: { baseURI: `https://mapgenie.io${PAGE}` },
    location: { pathname: PAGE },
    mapData,
  }) as unknown as Window;

describe('mapgenie user locations', () => {
  test('isUserLocationRequest', () => {
    expect(isUserLocationRequest('PUT', new URL(API))).toBe(true);
    expect(isUserLocationRequest('delete', new URL(`${API}/`))).toBe(true);
    expect(isUserLocationRequest('GET', new URL(API))).toBe(false);
    expect(isUserLocationRequest('POST', new URL(API))).toBe(false);
    expect(
      isUserLocationRequest(
        'PUT',
        new URL('https://cdn.mapgenie.io/api/v1/user/locations/1'),
      ),
    ).toBe(false);
    expect(
      isUserLocationRequest(
        'PUT',
        new URL('https://mapgenie.io/api/v1/user/locations/abc'),
      ),
    ).toBe(false);
  });

  test('mapId reads window.mapData.map.id', () => {
    expect(mapId(mapPage({ map: { id: 42 } }))).toBe('42');
    expect(mapId(mapPage({ map: { id: '42' } }))).toBe('42');
    expect(mapId(mapPage({ map: {} }))).toBeNull();
    expect(mapId(mapPage())).toBeNull();
  });

  test('headers for location requests only, relative URLs included', () => {
    const headers = userLocationHeaders(mapPage({ map: { id: 42 } }));
    const expected = { [MAP_ID_HEADER]: '42', [PAGE_PATH_HEADER]: PAGE };
    expect(headers('PUT', API)).toEqual(expected);
    expect(headers('DELETE', '/api/v1/user/locations/388128')).toEqual(
      expected,
    );
    expect(headers('GET', API)).toBeNull();
    expect(
      headers('PUT', 'https://mapgenie.io/api/v1/user/profiles/1'),
    ).toBeNull();
  });

  test('map-data requests get the page path only', () => {
    const headers = userLocationHeaders(mapPage({ map: { id: 42 } }));
    const pagePath = { [PAGE_PATH_HEADER]: PAGE };
    expect(
      headers('GET', 'https://mapgenie.io/api/v1/user/map-data/1092'),
    ).toEqual(pagePath);
    expect(headers('get', '/api/v1/user/map-data/1092')).toEqual(pagePath);
    expect(
      headers('POST', 'https://mapgenie.io/api/v1/user/map-data/1092'),
    ).toBeNull();
    expect(
      headers('GET', 'https://mapgenie.io/api/v1/user/map-data/abc'),
    ).toBeNull();
  });

  test('note calls get the page path only', () => {
    const headers = userLocationHeaders(mapPage({ map: { id: 42 } }));
    const pagePath = { [PAGE_PATH_HEADER]: PAGE };
    const notes = 'https://mapgenie.io/api/v1/user/notes';
    expect(headers('POST', notes)).toEqual(pagePath);
    expect(headers('PUT', `${notes}/H6ZdMwjEj2G`)).toEqual(pagePath);
    expect(headers('DELETE', `${notes}/H6ZdMwjEj2G?map_id=645`)).toEqual(
      pagePath,
    );
    expect(headers('GET', notes)).toBeNull();
    expect(headers('PUT', notes)).toBeNull();
    expect(headers('POST', `${notes}/H6ZdMwjEj2G`)).toBeNull();
    expect(headers('PUT', `${notes}/a/b`)).toBeNull();
  });

  test('without mapData, only the page path is sent', () => {
    expect(userLocationHeaders(mapPage())('PUT', API)).toEqual({
      [PAGE_PATH_HEADER]: PAGE,
    });
  });
});

describe('overrides add the headers', () => {
  const extraHeaders = userLocationHeaders(mapPage({ map: { id: 42 } }));

  test('fetch, keeping the headers of the request', async () => {
    const original = rs.fn(async () => new Response('{}'));
    const fetch = createFetch(
      original as unknown as typeof globalThis.fetch,
      null,
      extraHeaders,
    );

    await fetch(API, {
      method: 'PUT',
      headers: { 'Content-Type': 'application/json' },
    });
    await fetch(new Request(API, { method: 'DELETE' }));
    await fetch('https://mapgenie.io/api/v1/maps', { method: 'PUT' });

    const headersOf = (call: number) =>
      new Headers(
        (original.mock.calls[call] as [unknown, RequestInit | undefined])[1]
          ?.headers,
      );
    expect(headersOf(0).get(MAP_ID_HEADER)).toBe('42');
    expect(headersOf(0).get(PAGE_PATH_HEADER)).toBe(PAGE);
    expect(headersOf(0).get('Content-Type')).toBe('application/json');
    expect(headersOf(1).get(MAP_ID_HEADER)).toBe('42');
    expect(headersOf(2).get(MAP_ID_HEADER)).toBeNull();
  });

  describe('XMLHttpRequest', () => {
    class FakeXMLHttpRequest {
      headers: Record<string, string> = {};
      open() {}
      send() {}
      setRequestHeader(name: string, value: string) {
        this.headers[name] = value;
      }
    }
    const Fake = FakeXMLHttpRequest as unknown as typeof XMLHttpRequest;
    let XHR: typeof XMLHttpRequest;

    beforeEach(() => {
      XHR = createXMLHttpRequest(Fake, null, extraHeaders);
    });

    test('sets them in open', () => {
      const xhr = new XHR() as unknown as FakeXMLHttpRequest;
      (xhr as unknown as XMLHttpRequest).open('PUT', API);
      expect(xhr.headers).toEqual({
        [MAP_ID_HEADER]: '42',
        [PAGE_PATH_HEADER]: PAGE,
      });

      const other = new XHR() as unknown as FakeXMLHttpRequest;
      (other as unknown as XMLHttpRequest).open('GET', API);
      expect(other.headers).toEqual({});
    });
  });
});
