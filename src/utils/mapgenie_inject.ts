import './mapgenie_inject.css'

// biome-ignore lint/suspicious/noExplicitAny: cheat
const target = window as any

// biome-ignore lint/suspicious/noExplicitAny: cheat
target.injectInputs = (data: any) => {
  const keys = Object.keys(data);
  for (const i in keys) {
    const key = keys[i];
    if (key === 'user') {
      target.user = {
        id: 123456,
        role: "user",
        locations: {},
        gameLocationsCount: 0,
        trackedCategoryIds: [],
        suggestions: [],
        presets: [],

        ...(data.user || {}),
        
        hasPro: true,
      };
    } else {
      target[key] = data[key];
    }
  }

  const xhr = new XMLHttpRequest();
  xhr.open('GET', `/api/local/map-data/${target.game.id}/${target.mapData.map.id}`, false);
  xhr.send();

  if (xhr.status === 200) {
    const response = JSON.parse(xhr.responseText);
    for (const location of response.locations) {
      target.user.locations[location] = true;
    }
    target.mapData.notes = response.notes;
  }
}