import { afterEach, describe, expect, rs, test } from '@rstest/core';
import { trapGlobal } from '../src/inject/globals/trap';

const globals = window as unknown as Record<string, unknown>;
const names = ['__trapA', '__trapB', '__trapC', '__trapD', 'abc'];

afterEach(() => {
  for (const name of names) delete globals[name];
});

describe('trapGlobal', () => {
  test('sees what the page assigns later, and can replace it', () => {
    const seen: unknown[] = [];
    trapGlobal<{ n: number }>(window, '__trapA', (value) => {
      seen.push(value);
      return { n: value.n * 10 };
    });

    globals.__trapA = { n: 1 };
    expect(seen).toEqual([{ n: 1 }]);
    expect(globals.__trapA).toEqual({ n: 10 });
  });

  test('keeps the value when the callback returns undefined', () => {
    trapGlobal(window, '__trapB', () => undefined);
    const value = { kept: true };
    globals.__trapB = value;
    expect(globals.__trapB).toBe(value);
  });

  test('catches top-level `var` and sloppy assignments of classic scripts', () => {
    const seen: unknown[] = [];
    trapGlobal(window, '__trapC', (value) => {
      seen.push(value);
      return undefined;
    });

    // Indirect eval (through a variable) runs as global, non-strict code.
    // biome-ignore lint/security/noGlobalEval: emulates a classic inline <script> (global, non-strict code).
    const globalEval = eval;
    globalEval('var __trapC = 1; __trapC = 2;');
    expect(seen).toEqual([1, 2]);
    expect(globals.__trapC).toBe(2);
  });

  test('runs on an existing value, and skips non-configurable globals', () => {
    globals.__trapD = 'before';
    const onAssign = rs.fn(() => undefined);
    expect(trapGlobal(window, '__trapD', onAssign)).toBe(true);
    expect(onAssign).toHaveBeenCalledWith('before');

    expect(trapGlobal(window, 'undefined', () => undefined)).toBe(false);
  });
});
