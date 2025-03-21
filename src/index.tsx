import { render } from 'solid-js/web'
import App from './App'
import { initCSSRuntime } from '@master/css-runtime'
import config from './master.css'
import './utils/mapgenie_inject'
 
initCSSRuntime(config)

const root = document.getElementById('root')
if (root) {
  render(() => <App />, root);
}
