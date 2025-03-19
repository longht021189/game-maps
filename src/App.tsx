import AssistiveButton from './components/buttons/AssistiveButton'
import SettingsPage, { settingsPageVisibilitySignal } from './components/pages/settings/SettingsPage'

const App = () => {
  return (
    <div>
      {settingsPageVisibilitySignal[0]() && <SettingsPage />}
      <AssistiveButton />
    </div>
  )
}

export default App