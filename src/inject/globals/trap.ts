/**
 * Called with each value the page assigns to a trapped global. Returning a value stores that one
 * instead (to wrap or patch the page's object); returning `undefined` keeps the page's value.
 */
export type OnAssign<T> = (value: T) => T | undefined;

/**
 * Traps assignments to the global `name` of `win`, made by page scripts that run after this one
 * (initialization scripts run at document start).
 *
 * Caught: `window.name = …`, `name = …` and top-level `var name = …` (a `var` declaration keeps an
 * existing property, so its assignment goes through the setter). Not caught: top-level
 * `function name() {}` (replaces the property) and `let` / `const name` (not on `window`).
 *
 * Returns `false` when `name` cannot be trapped (an existing non-configurable property).
 */
export function trapGlobal<T>(
  win: Window,
  name: string,
  onAssign: OnAssign<T>,
): boolean {
  const target = win as unknown as Record<string, unknown>;
  const existing = Object.getOwnPropertyDescriptor(win, name);
  if (existing && !existing.configurable) return false;

  let current = existing ? (target[name] as T) : undefined;
  const assign = (value: T) => {
    const replaced = onAssign(value);
    current = replaced === undefined ? value : replaced;
  };
  if (existing) assign(current as T);

  Object.defineProperty(win, name, {
    configurable: true,
    enumerable: true,
    get: () => current,
    set: assign,
  });
  return true;
}
