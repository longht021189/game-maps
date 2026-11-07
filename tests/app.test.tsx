import { expect, test } from '@rstest/core';
import { render } from '@solidjs/web';
import App, { MAP_URL } from '../src/app/App';

test('loads mapgenie in a full-size iframe', () => {
  const root = document.createElement('div');
  document.body.append(root);
  const dispose = render(() => <App />, root);

  const iframe = root.querySelector('iframe');
  expect(iframe).toBeInTheDocument();
  expect(iframe?.getAttribute('src')).toBe(MAP_URL);

  dispose();
  root.remove();
});
