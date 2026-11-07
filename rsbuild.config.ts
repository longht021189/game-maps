import { defineConfig } from '@rsbuild/core';
import { pluginSolid } from '@rsbuild/plugin-solid';

// Set by `tauri android dev` / `tauri ios dev`: the address the device reaches the dev server on.
const host = process.env.TAURI_DEV_HOST;

// Docs: https://rsbuild.rs/config/
export default defineConfig({
  // Solid's JSX transform; the `inject` scripts have no JSX, so sharing it is harmless.
  plugins: [pluginSolid()],
  environments: {
    // The SolidJS frontend (`dist/`, `build.frontendDist` in `src-tauri/tauri.conf.json`).
    web: {
      source: {
        entry: { index: './src/entrypoint/index.tsx' },
      },
      html: {
        title: 'game-maps-app',
      },
    },
    // Initialization scripts embedded by `src-tauri` (`include_str!`): one self-contained file
    // each in `dist-inject/`, no HTML, no hash, no chunks.
    inject: {
      source: {
        entry: { 'http-bodies': './src/entrypoint/http-bodies.ts' },
      },
      output: {
        distPath: { root: 'dist-inject', js: '' },
        filenameHash: false,
        legalComments: 'none',
        sourceMap: false,
      },
      splitChunks: false,
      server: {
        publicDir: false,
      },
      tools: {
        htmlPlugin: false,
      },
    },
  },
  server: {
    // Must match `build.devUrl` in `src-tauri/tauri.conf.json`.
    port: 3000,
    strictPort: true,
    host: host || 'localhost',
  },
  dev: {
    client: host ? { host, port: 3000 } : undefined,
  },
});
