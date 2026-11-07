import { trapGlobal } from '../inject/globals/trap';

const getLocations = () => {
  try {
    const xhr = new XMLHttpRequest();
    xhr.open('GET', `${window.location.pathname}/locations`, false);
    xhr.send();
    return JSON.parse(xhr.responseText)
  } catch(e) {
    return {}
  }
}

export function trapMapGenie(win: Window): void {
  trapGlobal<Record<string, unknown> | null>(win, 'user', () => {
    return {
      ...getLocations(),
      "id": 9967907,
      "role": "user",
      "activeProfileId": null,
      // "trackedCategoryIds": [11336],
      "trackedCategoryIds": [],
      "suggestions": [],
      "profiles": []
    };
  });
}