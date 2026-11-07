import type { Handover } from './handover';
import { syncBytes } from './helpers';

/**
 * Hands over the data of `navigator.sendBeacon` (always `POST`). `Blob` / `FormData` data cannot
 * be read synchronously and is sent as is.
 */
export function patchSendBeacon(handover: Handover): void {
  const navigator = handover.win.navigator;
  if (!navigator.sendBeacon) return;
  const original = navigator.sendBeacon;

  navigator.sendBeacon = (url: string | URL, data?: BodyInit | null) => {
    const resolved = handover.resolve(url);
    const bytes = resolved ? syncBytes(data) : undefined;
    const target =
      resolved && bytes?.length ? handover.handOver(resolved, bytes) : url;
    return original.call(navigator, target, data);
  };
}
