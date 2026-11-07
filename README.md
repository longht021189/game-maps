# game-maps

A desktop and Android app for [mapgenie.io](https://mapgenie.io) interactive game maps, built with
[Tauri 2](https://tauri.app). The app has no ads, and it keeps your progress (found locations and
notes) on your device instead of in a mapgenie account.

## Purpose and disclaimer

This is a personal project for **learning and research**. It explores how a cross-platform
WebView app can intercept and answer real `http(s)` requests in Rust: WKWebView, WebView2 and the
Android WebView each handle this differently. mapgenie.io is used as a real-world test site. The
project is not meant to replace mapgenie or to compete with it.

- This project is **not affiliated with, endorsed by or connected to mapgenie.io** or its owners.
  "mapgenie" and the maps, data and content on the site belong to their respective owners.
- mapgenie's ads and Pro subscription fund the work behind the maps. **If you find the maps useful,
  please use the official site or app and consider buying a
  [Pro subscription](https://mapgenie.io)** to support the people who make them.
- The software is provided **"as is", without warranty of any kind**, under the MIT license. The
  authors are not responsible for how it is used. If you use it, you alone are responsible for
  following mapgenie's terms of service and the laws that apply to you.
- If you own rights to content this project touches and have a concern, please open an issue.

## How it works

The window is a full-screen `<iframe>` that loads `https://mapgenie.io/`. Every `http(s)` request
the WebView makes goes through Rust, not through the WebView's own network stack. The page keeps
its real origin (`https://mapgenie.io`).

- **HTTP interception.** The app uses a fork of [wry](https://github.com/tauri-apps/wry)
  ([longht021189/tauri-apps_wry](https://github.com/longht021189/tauri-apps_wry)) that can intercept
  real `http(s)` origins. Tauri registers a wildcard interceptor, and the Rust proxy
  (`src-tauri/src/proxy/`) fetches each request with `reqwest`. The proxy uses its own cookie jar
  and removes the headers that would stop mapgenie from loading in an iframe.
- **Ad blocking.** Requests to ad and tracking domains (`src-tauri/src/proxy/blocklist.rs`) never
  reach the network. They get an empty `204` response. Injected CSS hides the empty ad slots.
- **Local progress.** The app answers mapgenie's own user API in Rust, from a local
  [redb](https://github.com/cberner/redb) database. These requests never reach mapgenie:

  | Request | What the app does |
  |---|---|
  | `PUT` / `DELETE /api/v1/user/locations/<id>` | Marks a location as found / not found |
  | `GET /api/v1/user/map-data/<map id>` | Returns the stored locations and notes |
  | `POST /api/v1/user/notes`, `PUT` / `DELETE /api/v1/user/notes/<id>` | Creates, updates or deletes a note |

  Data is stored per map page, keyed by the page's URL path
  (`/phantom-blade-zero/maps/weeping-creek-valley`). An initialization script tells the proxy
  which page a request comes from (`src/inject/mapgenie/locations.ts`). The database file is
  `locations.redb` in the app data directory, for example
  `~/Library/Application Support/app.game-maps/` on macOS.
- **Initialization script.** `src/entrypoint/http-bodies.ts` runs in every frame before the
  page's own scripts. It wraps `fetch` and `XMLHttpRequest`. On Android, it also passes request
  bodies to wry, because Android's WebView does not give request bodies to the app.

## The wry fork

The HTTP interception comes from our fork of wry,
**[longht021189/tauri-apps_wry](https://github.com/longht021189/tauri-apps_wry)** (branch `main`).
The fork is not tied to this app: any wry or Tauri project can use it to serve a real `http(s)`
origin from Rust while the page keeps that origin. This lets you mock or proxy a site, cache it
offline, filter it, or rewrite it.

- `WebViewBuilder::with_http_interceptor(origin, handler)` intercepts one origin
  (`"https://app.com"`), or every origin with the wildcards `wry::ANY_HTTP_ORIGIN` /
  `wry::ANY_HTTPS_ORIGIN`. `with_http_interceptor_for_all(handler)` covers both schemes.
- An exact origin takes precedence over a custom protocol, and a custom protocol over a wildcard.
  A wildcard therefore never hides Tauri's own protocols, such as IPC.
- In Tauri: a custom protocol whose name contains `://` becomes an interceptor, so
  `register_asynchronous_uri_scheme_protocol(wry::ANY_HTTPS_ORIGIN, handler)` works without patching
  Tauri.
- `wry::is_http_interception_supported()` tells whether the current platform supports it: Android,
  Windows, macOS and iOS do.
- macOS / iOS: the WKWebView task follows a `3xx` returned by the handler, so the page and `fetch`
  end up at the new URL.
- Android: the WebView never gives request bodies to the app. The fork exposes a synchronous
  native bridge (`window.__wryHttpBodies`) so that an initialization script can pass the body
  along with the request.

To use it, patch `wry` in your `Cargo.toml`:

```toml
[patch.crates-io]
wry = { git = "https://github.com/longht021189/tauri-apps_wry.git", branch = "main" }
```

The fork follows upstream wry (`dev` tracks
[tauri-apps/wry](https://github.com/tauri-apps/wry)). Feedback and issues are welcome on the
fork's repository.

## Platforms

| Platform | Status |
|---|---|
| macOS | Supported |
| Windows | Supported (WebView2) |
| Android | Supported |
| iOS | Should work (same WebKit code as macOS), not tested |
| Linux | Not supported: WebKitGTK cannot intercept `http(s)`, so the app exits at startup |

## Requirements

- [Node.js](https://nodejs.org) 24 and [pnpm](https://pnpm.io) 12. Use pnpm, not npm or npx.
- [Rust](https://rustup.rs), stable.
- The [Tauri prerequisites](https://tauri.app/start/prerequisites/) for your OS.
- For Android: the Android SDK, JDK 17 or newer, and the NDK with `NDK_HOME` set, for example
  `export NDK_HOME="$ANDROID_HOME/ndk/30.0.16248370"`.

`wry` comes from the fork on GitHub (`[patch.crates-io]` in `src-tauri/Cargo.toml`). To build
against a local checkout instead, add a `[patch.crates-io]` entry in a `.cargo/config.toml` above
this repository. It takes precedence over the one in `Cargo.toml`.

## Development

```sh
pnpm install

pnpm tauri dev                  # desktop app, with the dev server on http://localhost:3000
pnpm tauri android dev          # Android emulator or device

pnpm test                       # frontend and init script tests (Rstest)
pnpm check                      # lint and format (Biome)
cd src-tauri && cargo test      # Rust tests
```

The initialization script is built separately (`pnpm build:inject`, into `dist-inject/`) and
embedded into the Rust binary at compile time. `pnpm tauri dev` and `pnpm tauri build` build it
first.

## Building

```sh
pnpm tauri build                          # desktop bundles for the current OS
pnpm tauri android build --apk            # Android APK
```

## Releases

The **Build** workflow (`.github/workflows/build.yml`) only runs by hand:

```sh
gh workflow run build.yml --ref dev -f version=1.2.3
```

It checks the version (`X.Y.Z`, tag not used yet) and builds three targets with that version:

- macOS: universal `.dmg`
- Windows: `.exe` (NSIS) and `.msi`
- Android: arm64 `.apk`

When all three succeed, it creates a **draft** release `v1.2.3` with the files attached. Publishing
the draft creates the tag on the commit that was built. Workflow artifacts are kept for 7 days.
Release files stay until the release is deleted.

The workflow signs the Android APK if these repository secrets are set: `ANDROID_KEYSTORE_BASE64`,
`ANDROID_KEYSTORE_PASSWORD`, `ANDROID_KEY_ALIAS` and `ANDROID_KEY_PASSWORD`. Otherwise the APK is
unsigned. The macOS and Windows builds are not code-signed.

## Project layout

| Path | Contents |
|---|---|
| `src/app/` | SolidJS UI (the `<iframe>`) |
| `src/entrypoint/` | Rsbuild entries: `index.tsx` (UI), `http-bodies.ts` (initialization script) |
| `src/inject/http-bodies/` | `fetch` / `XMLHttpRequest` / beacon / form overrides, and the Android body handover |
| `src/inject/mapgenie/` | CSS that hides ad slots, and the headers for the user API |
| `src/inject/globals/` | Helper to trap globals that page scripts assign |
| `src-tauri/src/proxy/` | Rust proxy: forwarding, header cleanup, ad blocklist, user locations and notes |
| `src-tauri/src/locations.rs` | redb store |
| `src-tauri/gen/android/` | Android project (package `app.game_maps`) |
| `tests/` | Rstest tests |

## License

[MIT](LICENSE). The license covers this project's code only, not mapgenie's content or data.
