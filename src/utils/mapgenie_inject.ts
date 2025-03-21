import './mapgenie_inject.css'

// biome-ignore lint/suspicious/noExplicitAny: cheat
const target = window as any

target.injectInputs = () => {
  target.user = {
    id: 123456,
    role: "user",
    locations: {},
    gameLocationsCount: 0,
    trackedCategoryIds: [],
    suggestions: [],
    presets: [],
    ...(target.user || {}),
    hasPro: true,
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