import { createSignal } from "solid-js"
import { invoke } from '@tauri-apps/api/core'

export const settingsPageVisibilitySignal = createSignal(false)

const SettingsPage = () => {
  return (
    <div class="w:full h:full background:white">
      SettingsPage
    </div>
  )
}

export default SettingsPage