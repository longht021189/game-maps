import css from './styles.css?raw';

/** Whether `hostname` is mapgenie.io or one of its subdomains. */
export function isMapgenie(hostname: string): boolean {
  const host = hostname.toLowerCase();
  return host === 'mapgenie.io' || host.endsWith('.mapgenie.io');
}

/**
 * Adds the mapgenie style overrides (`./styles.css`) to `doc`.
 *
 * Runs at document start, possibly before `<head>` (or even `<html>`) exists: the `<style>` goes
 * into the document element as soon as there is one.
 */
export function injectStyles(doc: Document): HTMLStyleElement {
  const style = doc.createElement('style');
  style.dataset.gameMaps = 'mapgenie';
  style.textContent = css;

  const append = () => {
    const parent = doc.head ?? doc.documentElement;
    if (!parent) return false;
    parent.append(style);
    return true;
  };

  if (!append()) {
    const observer = new MutationObserver(() => {
      if (append()) observer.disconnect();
    });
    observer.observe(doc, { childList: true });
  }
  return style;
}
