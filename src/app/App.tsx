import './App.css';

/** Page shown inside the app. Requests to it go through the Rust proxy (see `src-tauri/src/proxy`). */
export const MAP_URL = 'https://mapgenie.io/';

const App = () => {
  return <iframe class="map" src={MAP_URL} title="MapGenie" />;
};

export default App;
