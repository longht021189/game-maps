import type { Handover } from './handover';
import { formBytes, hasBody } from './helpers';

/**
 * Hands over the body of form submissions (url-encoded and plain text; multipart boundaries are
 * chosen by the browser). Submissions are navigations, so each one is pointed at the tagged URL.
 */
export function patchForms(handover: Handover): void {
  const win = handover.win;

  const prepare = (form: HTMLFormElement, submitter: HTMLElement | null) => {
    const method = submitter?.getAttribute('formmethod') || form.method;
    if (!hasBody(method) || method.toLowerCase() === 'dialog') return;

    const action =
      (submitter instanceof win.HTMLButtonElement ||
      submitter instanceof win.HTMLInputElement
        ? submitter.formAction
        : '') || form.action;
    const url = handover.resolve(action);
    const bytes = url ? formBytes(form, submitter) : undefined;
    if (!url || !bytes) return;

    const tagged = handover.handOver(url, bytes);
    const target = submitter?.hasAttribute('formaction') ? submitter : form;
    const attribute = target === form ? 'action' : 'formaction';
    const previous = target.getAttribute(attribute);
    target.setAttribute(attribute, tagged);
    win.setTimeout(() => {
      if (previous == null) target.removeAttribute(attribute);
      else target.setAttribute(attribute, previous);
    });
  };

  // `submit` fires for user submissions and `requestSubmit()`. Listening on `window` (bubbling)
  // runs after the page's own handlers, so a cancelled submission leaves no body behind.
  win.addEventListener('submit', (event) => {
    if (
      !event.defaultPrevented &&
      event.target instanceof win.HTMLFormElement
    ) {
      prepare(event.target, event.submitter);
    }
  });

  // `form.submit()` fires no event.
  const originalSubmit = win.HTMLFormElement.prototype.submit;
  win.HTMLFormElement.prototype.submit = function (this: HTMLFormElement) {
    prepare(this, null);
    return originalSubmit.call(this);
  };
}
