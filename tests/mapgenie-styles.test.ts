import { expect, test } from '@rstest/core';
import { injectStyles, isMapgenie } from '../src/inject/mapgenie/styles';

test('isMapgenie', () => {
  expect(isMapgenie('mapgenie.io')).toBe(true);
  expect(isMapgenie('cdn.MapGenie.io')).toBe(true);
  expect(isMapgenie('notmapgenie.io')).toBe(false);
  expect(isMapgenie('mapgenie.io.example.com')).toBe(false);
});

test('hides the ad slots and the room kept for them', () => {
  const style = injectStyles(document);
  try {
    expect(style.isConnected).toBe(true);
    expect(style.textContent).toContain('.billboard');
    expect(style.textContent).toContain('#blobby-left');
    expect(style.textContent).toContain(
      '#left-sidebar.footer-large #categories',
    );
    expect(style.textContent).toContain(
      '#left-sidebar.footer-large #search-results-wrapper',
    );

    document.body.innerHTML =
      '<div class="billboard"></div><div id="blobby-left"></div>';
    for (const element of document.body.children) {
      expect(getComputedStyle(element).display).toBe('none');
    }
  } finally {
    style.remove();
    document.body.innerHTML = '';
  }
});

test('waits for the document element when there is none yet', async () => {
  const doc = document.implementation.createHTMLDocument();
  doc.documentElement.remove();
  const style = injectStyles(doc);
  expect(style.parentNode).toBeNull();

  doc.append(doc.createElement('html'));
  await new Promise((resolve) => setTimeout(resolve, 0));
  expect(style.parentNode).toBe(doc.documentElement);
});
