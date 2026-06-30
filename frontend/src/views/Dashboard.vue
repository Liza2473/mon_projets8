<template>
  <div class="dashboard" :data-theme="isDark ? 'dark' : 'light'">

    <!-- Header -->
    <div class="dash-header">
      <div class="header-left">
        <div class="header-logo">
          <span class="header-pulse"></span>
          IoT Controller
        </div>
        <span v-if="backendStatus === 'Connecté'" class="badge-live">
          <span class="live-dot"></span> LIVE
        </span>
        <span v-if="isOffline" class="badge-offline">OFFLINE</span>
        <span class="header-clock">🕐 {{ currentTime }}</span>
      </div>
      <div class="header-right">
        <button class="theme-toggle" @click="toggleTheme">{{ isDark ? '☀️' : '🌙' }}</button>
        <button class="hbtn-disconnect" @click="toggleConnection">
          {{ backendStatus === 'Connecté' ? 'Se déconnecter' : 'Se connecter' }}
        </button>
      </div>
    </div>

    <!-- Barre de statut industrielle -->
    <div class="status-bar">
      <div class="status-item">
        <span class="status-dot" :class="backendStatus === 'Connecté' ? 'dot-green' : 'dot-red'"></span>
        <span class="status-label">Backend</span>
        <span class="status-val" :class="backendStatus === 'Connecté' ? 'val-green' : 'val-red'">{{ backendStatus }}</span>
      </div>
      <div class="status-sep">|</div>
      <div class="status-item">
        <span class="status-dot" :class="backendStatus === 'Connecté' ? 'dot-green' : 'dot-red'"></span>
        <span class="status-label">MQTT</span>
        <span class="status-val" :class="backendStatus === 'Connecté' ? 'val-green' : 'val-red'">
          {{ backendStatus === 'Connecté' ? 'Actif' : 'Inactif' }}
        </span>
      </div>
      <div class="status-sep">|</div>
      <div class="status-item">
        <span class="status-label">🖥</span>
        <span class="status-val val-blue">{{ deviceId || '—' }}</span>
      </div>
      <div class="status-sep">|</div>
      <div class="status-item">
        <span class="status-label">🕒</span>
        <span class="status-val">{{ timeSinceUpdate }}</span>
      </div>
      <div class="status-sep">|</div>
      <div class="status-item">
        <span class="status-label">📈</span>
        <span class="status-val val-blue">{{ totalMeasures }} mesures</span>
      </div>
      <div class="status-sep">|</div>
      <div class="status-backend">
        <span class="url-label-sm">Backend :</span>
        <input v-model="backendUrlInput" class="url-input-sm" placeholder="http://localhost:3000" @keyup.enter="applyBackendUrl" />
        <button class="url-btn-sm" @click="applyBackendUrl">Changer</button>
      </div>
      <div class="status-actions">
        <button class="cmd-action-btn help-btn" @click="showHelp = true">❓ Aide</button>
        <button class="cmd-action-btn config-btn" @click="showConfig = true">⚙️ Configurer</button>
      </div>
    </div>

    <HelpPanel :is-open="showHelp" :flux-metadata="fluxMetadata" :device-id="deviceId" @close="showHelp = false" />
    <CommandConfig :is-open="showConfig" :flux-metadata="fluxMetadata" @close="showConfig = false" @update="onConfigUpdate" />

    <div v-if="isOffline" class="offline-banner">
      ⚠️ Mode offline — affichage des dernières données reçues
    </div>

    <!-- Layout -->
    <div class="main-layout">

      <!-- COLONNE GAUCHE -->
      <div class="col">

        <!-- Données IoT -->
        <div class="card">
          <div class="card-header">
            <h3 class="section-title"><span class="title-dot"></span>Données IoT</h3>
            <span :class="backendStatus === 'Connecté' ? 'badge-connected' : 'badge-disconnected'">
              {{ backendStatus }}
            </span>
          </div>

          <div class="data-values">
            <div
              v-for="flux in fluxMetadata" :key="flux.id"
              class="data-value-card"
              :class="{ disabled: !activeFlux[flux.id] }"
              :style="flux.id === 'pression'
                ? { borderColor: `${presColor}60`, background: `${presColor}15` }
                : { borderColor: `${tempColor}60`, background: `${tempColor}15` }"
            >
              <div class="flux-icon">
                {{ flux.id === 'temperature' ? '🌡️' : flux.id === 'pression' ? '🔵' : '📡' }}
              </div>
              <span class="data-value-number" :style="{ color: flux.id === 'temperature' ? tempColor : presColor }">
                {{ fluxValues[flux.id] !== undefined ? fluxValues[flux.id] : '--' }}
              </span>
              <span class="data-value-unit">{{ flux.unit }}</span>
              <span class="data-value-label">{{ flux.label }}</span>
            </div>
            <div v-if="fluxMetadata.length === 0" class="data-value-card disabled">
              <div class="flux-icon">📡</div>
              <span class="data-value-number" style="color:var(--text-muted)">--</span>
              <span class="data-value-unit">En attente...</span>
            </div>
          </div>

          <div class="freshness-bar">
            <div class="freshness-label">{{ lastMessage }}</div>
            <div class="freshness-track">
              <div class="freshness-fill" :style="{ width: freshnessPercent + '%', background: freshnessColor }"></div>
            </div>
          </div>

          <!-- Badges Min/Moy/Max -->
          <div v-if="tempData.length > 0 || pressionData.length > 0" class="stats-modern">
            <div v-if="tempData.length > 0" class="stats-modern-row">
              <span class="stats-modern-title" :style="{ color: tempColor }">🌡️ Température</span>
              <div class="stats-modern-badges">
                <span class="stat-badge stat-min">
                  <span class="stat-badge-label">MIN</span>
                  <span class="stat-badge-val" :style="{ color: tempColor }">{{ tempMin }}°</span>
                </span>
                <span class="stat-badge stat-avg">
                  <span class="stat-badge-label">MOY</span>
                  <span class="stat-badge-val" :style="{ color: tempColor }">{{ tempAvg }}°</span>
                </span>
                <span class="stat-badge stat-max">
                  <span class="stat-badge-label">MAX</span>
                  <span class="stat-badge-val" :style="{ color: tempColor }">{{ tempMax }}°</span>
                </span>
              </div>
            </div>
            <div v-if="pressionData.length > 0" class="stats-modern-row">
              <span class="stats-modern-title" :style="{ color: presColor }">🔵 Pression</span>
              <div class="stats-modern-badges">
                <span class="stat-badge stat-min">
                  <span class="stat-badge-label">MIN</span>
                  <span class="stat-badge-val" :style="{ color: presColor }">{{ presMin }}</span>
                </span>
                <span class="stat-badge stat-avg">
                  <span class="stat-badge-label">MOY</span>
                  <span class="stat-badge-val" :style="{ color: presColor }">{{ presAvg }}</span>
                </span>
                <span class="stat-badge stat-max">
                  <span class="stat-badge-label">MAX</span>
                  <span class="stat-badge-val" :style="{ color: presColor }">{{ presMax }}</span>
                </span>
              </div>
            </div>
          </div>
        </div>

        <!-- Flux disponibles -->
        <div class="card">
          <h3 class="section-title"><span class="title-dot"></span>Flux disponibles</h3>
          <div v-if="fluxMetadata.length > 0" class="pins">
            <button
              v-for="flux in fluxMetadata" :key="flux.id"
              type="button" class="pin-card"
              :class="{ active: activeFlux[flux.id] }"
              @click.stop="toggleFlux(flux.id)"
            >
              <div class="icon">{{ flux.id.substring(0, 2).toUpperCase() }}</div>
              <p>{{ flux.label }}</p>
              <small class="flux-desc">{{ flux.desc }}</small>
            </button>
          </div>
          <div v-else class="waiting-metadata"><p>⏳ En attente des flux...</p></div>
          <p v-if="deviceId" class="detected-info">Appareil : <span class="detected-list">{{ deviceId }}</span></p>
        </div>

        <!-- État système enrichi -->
        <div class="card card-grow">
          <h3 class="section-title"><span class="title-dot"></span>État système</h3>
          <div class="system-rows">
            <div class="system-row">
              <span class="sys-label">Backend</span>
              <span class="sys-val">
                <span class="sys-dot" :class="backendStatus === 'Connecté' ? 'dot-green' : 'dot-red'"></span>
                <span :class="backendStatus === 'Connecté' ? 'sys-green' : 'sys-red'">{{ backendStatus }}</span>
              </span>
            </div>
            <div class="system-row">
              <span class="sys-label">MQTT</span>
              <span class="sys-val">
                <span class="sys-dot" :class="backendStatus === 'Connecté' ? 'dot-green' : 'dot-red'"></span>
                <span :class="backendStatus === 'Connecté' ? 'sys-green' : 'sys-red'">
                  {{ backendStatus === 'Connecté' ? 'Actif' : 'Inactif' }}
                </span>
              </span>
            </div>
            <div class="system-row">
              <span class="sys-label">Appareil</span>
              <span class="sys-val sys-blue">{{ deviceId || '—' }}</span>
            </div>
            <div class="system-row">
              <span class="sys-label">Dernier flux</span>
              <span class="sys-val">{{ timeSinceUpdate }}</span>
            </div>
            <div class="system-row">
              <span class="sys-label">Mesures</span>
              <span class="sys-val sys-blue">{{ totalMeasures }}</span>
            </div>
            <div class="system-row">
              <span class="sys-label">Uptime</span>
              <span class="sys-val sys-green">{{ uptimeFormatted }}</span>
            </div>
            <div class="system-row">
              <span class="sys-label">Commandes</span>
              <span class="sys-val sys-blue">{{ commandsSent }}</span>
            </div>
            <div class="system-row">
              <span class="sys-label">Dernier ordre</span>
              <span class="sys-val" style="font-family:monospace;font-size:11px">{{ lastCommandSent }}</span>
            </div>
          </div>
        </div>

      </div>

      <!-- COLONNE CENTRE -->
      <div class="col col-center">

        <!-- OLED -->
        <div class="card">
          <h3 class="section-title"><span class="title-dot"></span>Simulation OLED — 128×64</h3>
          <OledScreen
            :mode="oledMode"
            :temperature="fluxValues['temperature'] ?? '--'"
            :pression="fluxValues['pression'] ?? '--'"
            :response-line1="oledResponse1"
            :response-line2="oledResponse2"
            :response-line3="oledResponse3"
            :response-ok="oledResponseOk"
            :last-command="lastOledCommand"
            @send-message="sendOledMessage"
          />
        </div>

        <!-- Graphique -->
        <div class="card card-grow" :class="{ 'offline-alert': isOffline }">
          <h3 class="section-title"><span class="title-dot"></span>Graphique temps réel</h3>
          <ChartPanel
            :temperature="tempDataFiltered"
            :humidity="pressionDataFiltered"
            :timestamps="timestamps"
          />
        </div>

      </div>

      <!-- COLONNE DROITE -->
      <div class="col">

        <!-- Commandes -->
        <div class="card cmd-card">
          <h3 class="section-title"><span class="title-dot"></span>Commandes</h3>
          <div class="cmd-list">
            <div v-if="generalCommands.length === 0 && fluxCommands.length === 0 && customCommandsVisible.length === 0" class="waiting-metadata">
              <p>⏳ En attente...</p>
            </div>

            <!-- Générales -->
            <div v-if="generalCommands.length > 0" class="cmd-section-label general-label">⚙️ Générales</div>
            <button
              v-for="cmd in generalCommands" :key="cmd.id"
              class="cmd-btn" :class="{ active: activeCommand === cmd.id }"
              :title="cmd.label"
              @click="sendCommand(cmd.id)"
            >
              <span class="cmd-tag general-tag">{{ cmd.id }}</span>
              <span class="cmd-desc">{{ cmd.label }}</span>
            </button>

            <!-- Flux Arduino -->
            <div v-if="fluxCommands.length > 0" class="cmd-section-label flux-label">📡 Flux</div>
            <button
              v-for="cmd in fluxCommands" :key="cmd.id"
              class="cmd-btn" :class="{ active: activeCommand === cmd.id }"
              :title="cmd.label"
              @click="sendCommand(cmd.id)"
            >
              <span class="cmd-tag flux-tag">{{ cmd.id }}</span>
              <span class="cmd-desc">{{ cmd.label }}</span>
            </button>

            <!-- ✅ Commandes personnalisées -->
            <div v-if="customCommandsVisible.length > 0" class="cmd-section-label custom-label">✏️ Personnalisées</div>
            <button
              v-for="cmd in customCommandsVisible" :key="cmd.id"
              class="cmd-btn" :class="{ active: activeCommand === cmd.id }"
              :title="cmd.label"
              @click="sendCommand(cmd.id)"
            >
              <span class="cmd-tag custom-tag">{{ cmd.id }}</span>
              <span class="cmd-desc">{{ cmd.label }}</span>
            </button>
          </div>
        </div>

        <!-- Console rétractable + plein écran -->
        <div class="card console-card" :class="{ collapsed: consoleCollapsed }">
          <div class="console-header" @click="consoleCollapsed = !consoleCollapsed">
            <h3 class="section-title" style="margin-bottom:0">
              <span class="title-dot"></span>Console
            </h3>
            <div class="console-header-actions">
              <button
                class="console-fullscreen-btn"
                @click.stop="consoleFullscreen = !consoleFullscreen"
                :title="consoleFullscreen ? 'Réduire' : 'Plein écran'"
              >{{ consoleFullscreen ? '⊠' : '⛶' }}</button>
              <span class="console-toggle">{{ consoleCollapsed ? '▼' : '▲' }}</span>
            </div>
          </div>
          <div v-show="!consoleCollapsed" class="console-body">
            <ConsolePanel :logs="logs" @send-command="sendCommand" @clear-console="clearConsole" />
          </div>
        </div>

      </div>
    </div>

    <!-- Overlay console plein écran -->
    <div v-if="consoleFullscreen" class="console-overlay" @click.self="consoleFullscreen = false">
      <div class="console-overlay-panel">
        <div class="console-overlay-header">
          <h3 class="section-title" style="margin-bottom:0;color:#e6edf3">
            <span class="title-dot"></span>Console — Plein écran
          </h3>
          <button class="console-close-btn" @click="consoleFullscreen = false">✕ Fermer</button>
        </div>
        <div class="console-overlay-body">
          <ConsolePanel :logs="logs" @send-command="sendCommand" @clear-console="clearConsole" />
        </div>
      </div>
    </div>

    <FooterBar :backend-url="backendUrl" :backend-status="backendStatus" />
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onBeforeUnmount } from 'vue'
import { io } from 'socket.io-client'

import ConsolePanel  from '../components/ConsolePanel.vue'
import ChartPanel    from '../components/ChartPanel.vue'
import FooterBar     from '../components/FooterBar.vue'
import OledScreen    from '../components/OledScreen.vue'
import CommandConfig from '../components/CommandConfig.vue'
import HelpPanel     from '../components/HelpPanel.vue'

// Thème
const isDark = ref(true)
function applyTheme(dark) {
  document.documentElement.setAttribute('data-theme', dark ? 'dark' : 'light')
  document.body.style.background = dark
    ? 'radial-gradient(circle at top, #161b22, #0d1117)'
    : 'linear-gradient(135deg, #eef2f7, #dde4ef)'
}
function toggleTheme() {
  isDark.value = !isDark.value
  applyTheme(isDark.value)
  localStorage.setItem('theme', isDark.value ? 'dark' : 'light')
}

// Horloge
const currentTime = ref('')
function updateClock() {
  currentTime.value = new Date().toLocaleTimeString('fr-FR', {
    hour: '2-digit', minute: '2-digit', second: '2-digit'
  })
}

// Console
const consoleCollapsed  = ref(false)
const consoleFullscreen = ref(false)

// Backend
const BACKEND_URL_KEY     = 'backend-url'
const DEFAULT_BACKEND_URL = 'http://localhost:3000'
const backendUrl          = ref(localStorage.getItem(BACKEND_URL_KEY) || DEFAULT_BACKEND_URL)
const backendUrlInput     = ref(backendUrl.value)

// Etats
const logs         = ref([])
const tempData     = ref([])
const pressionData = ref([])
const timestamps   = ref([])
const lastMessage   = ref('Aucun message reçu')
const backendStatus = ref('Déconnecté')
const isOffline     = ref(!navigator.onLine)
const deviceId      = ref('')
const totalMeasures = ref(0)

// Uptime + commandes
const uptimeSeconds   = ref(0)
const commandsSent    = ref(0)
const lastCommandSent = ref('—')
let uptimeInterval    = null

const uptimeFormatted = computed(() => {
  const s = uptimeSeconds.value
  if (s < 60)   return `${s}s`
  if (s < 3600) return `${Math.floor(s/60)}m ${s%60}s`
  return `${Math.floor(s/3600)}h ${Math.floor((s%3600)/60)}m`
})

const oledMode        = ref('ALL')
const oledResponse1   = ref('')
const oledResponse2   = ref('')
const oledResponse3   = ref('')
const oledResponseOk  = ref(false)
const lastOledCommand = ref('—')
const showConfig      = ref(false)
const showHelp        = ref(false)
const activeCommand   = ref('')

// Metadata
const fluxMetadata = ref([])
const fluxValues   = ref({})
const activeFlux   = ref({})

const savedMeta = localStorage.getItem('flux-metadata')
if (savedMeta) {
  fluxMetadata.value = JSON.parse(savedMeta)
  fluxMetadata.value.forEach(f => { activeFlux.value[f.id] = true })
}
const savedValues = localStorage.getItem('flux-values')
if (savedValues) fluxValues.value = JSON.parse(savedValues)

// Couleurs dynamiques
const tempColor = computed(() => {
  const v = Number(fluxValues.value['temperature'])
  if (isNaN(v)) return '#38bdf8'
  if (v < 20)   return '#38bdf8'
  if (v < 30)   return '#22c55e'
  return '#ef4444'
})

const presColor = computed(() => {
  const v = Number(fluxValues.value['pression'])
  if (isNaN(v)) return '#34d399'
  if (v < 980)  return '#f0883e'
  if (v < 1020) return '#34d399'
  return '#38bdf8'
})

// Fraîcheur
const lastUpdateTime  = ref(null)
const timeSinceUpdate = ref('Aucune donnée reçue')

const freshnessPercent = computed(() => {
  if (!lastUpdateTime.value) return 0
  const s = Math.floor((Date.now() - lastUpdateTime.value) / 1000)
  return Math.max(0, Math.min(100, 100 - (s / 30) * 100))
})

const freshnessColor = computed(() => {
  if (freshnessPercent.value > 60) return '#22c55e'
  if (freshnessPercent.value > 30) return '#f0883e'
  return '#ef4444'
})

// Stats
const tempMin = computed(() => !tempData.value.length ? '--' : Math.min(...tempData.value).toFixed(1))
const tempMax = computed(() => !tempData.value.length ? '--' : Math.max(...tempData.value).toFixed(1))
const tempAvg = computed(() => !tempData.value.length ? '--' : (tempData.value.reduce((a,b)=>a+b,0)/tempData.value.length).toFixed(1))
const presMin = computed(() => !pressionData.value.length ? '--' : Math.min(...pressionData.value).toFixed(1))
const presMax = computed(() => !pressionData.value.length ? '--' : Math.max(...pressionData.value).toFixed(1))
const presAvg = computed(() => !pressionData.value.length ? '--' : (pressionData.value.reduce((a,b)=>a+b,0)/pressionData.value.length).toFixed(1))

// ============================================================
// Commandes
// ============================================================
const selectedCommands  = ref([])

// ✅ Custom commands — ref réactif mis à jour par onConfigUpdate
const customCommandsList = ref(
  JSON.parse(localStorage.getItem('custom-commands') || '[]')
)

const GENERAL_COMMANDS = [
  { id: 'ALL',      label: 'Affiche tout' },
  { id: 'DESCRIBE', label: 'Rafraîchir les flux' },
  { id: 'LIST',     label: 'Liste les flux' },
  { id: 'STATUS',   label: 'État des flux' },
]

function loadSelectedCommands() {
  const saved = localStorage.getItem('selected-commands-v2')
  selectedCommands.value = saved ? JSON.parse(saved) : GENERAL_COMMANDS.map(c => c.id)
}
loadSelectedCommands()

// Commandes générales filtrées
const generalCommands = computed(() =>
  GENERAL_COMMANDS
    .filter(c => selectedCommands.value.includes(c.id))
    .sort((a, b) => a.id.localeCompare(b.id))
)

// Commandes flux Arduino filtrées
const fluxCommands = computed(() => {
  const cmds = []
  fluxMetadata.value.forEach(flux => {
    const id = flux.id.toUpperCase()
    cmds.push({ id, label: flux.label })
    cmds.push({ id: id + '?', label: flux.label + ' existe ?' })
  })
  return cmds
    .filter(c => selectedCommands.value.includes(c.id))
    .sort((a, b) => a.id.localeCompare(b.id))
})

// ✅ Commandes personnalisées visibles (sélectionnées)
const customCommandsVisible = computed(() =>
  customCommandsList.value
    .filter(id => selectedCommands.value.includes(id))
    .map(id => ({ id, label: 'Commande personnalisée' }))
    .sort((a, b) => a.id.localeCompare(b.id))
)

// ✅ onConfigUpdate — met à jour selected + custom
function onConfigUpdate(data) {
  selectedCommands.value = data.selected
  if (data.custom) {
    customCommandsList.value = data.custom
    localStorage.setItem('custom-commands', JSON.stringify(data.custom))
  }
  addLog('⚙️ Configuration mise à jour')
}

// Historique
const savedTempHistory  = localStorage.getItem('temp-history')
const savedPressHistory = localStorage.getItem('pression-history')
const savedTimestamps   = localStorage.getItem('timestamps')
if (savedTempHistory)  tempData.value     = JSON.parse(savedTempHistory)
if (savedPressHistory) pressionData.value = JSON.parse(savedPressHistory)
if (savedTimestamps)   timestamps.value   = JSON.parse(savedTimestamps)

const tempDataFiltered     = computed(() => activeFlux.value['temperature'] ? tempData.value : [])
const pressionDataFiltered = computed(() => activeFlux.value['pression']    ? pressionData.value : [])

function updateTimeSince() {
  if (!lastUpdateTime.value) { timeSinceUpdate.value = 'Aucune donnée reçue'; return }
  const s = Math.floor((Date.now() - lastUpdateTime.value) / 1000)
  if (s < 5)       timeSinceUpdate.value = "à l'instant"
  else if (s < 60) timeSinceUpdate.value = `il y a ${s} sec`
  else             timeSinceUpdate.value = `il y a ${Math.floor(s / 60)} min`
}

function addLog(msg) {
  logs.value.push(msg)
  if (logs.value.length > 30) logs.value.shift()
}

function toggleFlux(fluxId) {
  activeFlux.value[fluxId] = !activeFlux.value[fluxId]
  addLog(activeFlux.value[fluxId] ? `✅ Flux ${fluxId} activé` : `⏸️ Flux ${fluxId} désactivé`)
}

// Socket
function createSocket(url) {
  const s = io(url, { autoConnect: false })

  s.on('connect', () => {
    backendStatus.value = 'Connecté'
    addLog('✅ Frontend connecté au backend (' + url + ')')
    uptimeSeconds.value = 0
    if (uptimeInterval) clearInterval(uptimeInterval)
    uptimeInterval = setInterval(() => { uptimeSeconds.value++ }, 1000)
    setTimeout(() => { s.emit('command', { action: 'DESCRIBE' }) }, 1000)
  })

  s.on('disconnect', () => {
    backendStatus.value = 'Déconnecté'
    addLog('❌ Frontend déconnecté du backend')
    if (uptimeInterval) clearInterval(uptimeInterval)
  })

  s.on('flux-metadata', (data) => {
    addLog('📋 Metadata reçue : ' + data.flows.map(f => f.id).join(', '))
    deviceId.value     = data.id
    fluxMetadata.value = data.flows
    data.flows.forEach(f => { if (activeFlux.value[f.id] === undefined) activeFlux.value[f.id] = true })
    localStorage.setItem('flux-metadata', JSON.stringify(data.flows))
    const saved = localStorage.getItem('selected-commands-v2')
    if (!saved) {
      selectedCommands.value = [
        ...GENERAL_COMMANDS.map(c => c.id),
        ...data.flows.flatMap(f => [f.id.toUpperCase(), f.id.toUpperCase() + '?'])
      ]
    }
  })

  s.onAny((eventName, data) => {
    if (!eventName.endsWith('-data')) return
    const fluxId = eventName.replace('-data', '')
    const value  = Number(data.value)
    const unit   = data.unit

    fluxValues.value[fluxId] = value
    localStorage.setItem('flux-values', JSON.stringify(fluxValues.value))
    lastMessage.value    = `${fluxId} : ${value} ${unit}`
    lastUpdateTime.value = Date.now()
    totalMeasures.value++
    addLog(`📡 ${fluxId} : ${value} ${unit}`)

    const now = new Date().toLocaleTimeString('fr-FR', {
      hour: '2-digit', minute: '2-digit', second: '2-digit'
    })

    if (fluxId === 'temperature' && activeFlux.value['temperature']) {
      tempData.value.push(value)
      timestamps.value.push(now)
      if (tempData.value.length  > 50) tempData.value.shift()
      if (timestamps.value.length > 50) timestamps.value.shift()
      localStorage.setItem('temp-history', JSON.stringify(tempData.value))
      localStorage.setItem('timestamps',   JSON.stringify(timestamps.value))
    }
    if (fluxId === 'pression' && activeFlux.value['pression']) {
      pressionData.value.push(value)
      if (pressionData.value.length > 50) pressionData.value.shift()
      localStorage.setItem('pression-history', JSON.stringify(pressionData.value))
    }
  })

  return s
}

let socket = createSocket(backendUrl.value)

function changeBackendUrl(newUrl) {
  socket.removeAllListeners()
  socket.disconnect()
  backendStatus.value = 'Déconnecté'
  backendUrl.value    = newUrl
  localStorage.setItem(BACKEND_URL_KEY, newUrl)
  socket = createSocket(newUrl)
  if (!isOffline.value) socket.connect()
  addLog('🔄 Adresse backend changée : ' + newUrl)
}

function applyBackendUrl() {
  const newUrl = backendUrlInput.value.trim()
  if (!newUrl || !newUrl.startsWith('http') || newUrl === backendUrl.value) return
  changeBackendUrl(newUrl)
}

function handleOnline()  { isOffline.value = false; addLog('✅ Connexion rétablie'); socket.connect() }
function handleOffline() { isOffline.value = true;  addLog('⚠️ Mode offline');       socket.disconnect() }

let timeInterval  = null
let clockInterval = null

onMounted(() => {
  const saved = localStorage.getItem('theme') || 'dark'
  isDark.value = saved === 'dark'
  applyTheme(isDark.value)
  updateClock()
  clockInterval = setInterval(updateClock, 1000)
  window.addEventListener('online',  handleOnline)
  window.addEventListener('offline', handleOffline)
  if (navigator.onLine) socket.connect()
  timeInterval = setInterval(updateTimeSince, 1000)
})

onBeforeUnmount(() => {
  window.removeEventListener('online',  handleOnline)
  window.removeEventListener('offline', handleOffline)
  if (timeInterval)   clearInterval(timeInterval)
  if (clockInterval)  clearInterval(clockInterval)
  if (uptimeInterval) clearInterval(uptimeInterval)
})

function toggleConnection() {
  if (socket.connected) { socket.disconnect(); addLog('🔌 Déconnexion demandée') }
  else                  { socket.connect();    addLog('🔌 Connexion demandée') }
}

function sendCommand(cmd) {
  if (isOffline.value || !socket.connected) { addLog('⚠️ Impossible : application offline'); return }
  const c = cmd.toUpperCase().trim()
  if (activeCommand.value === c) { activeCommand.value = ''; return }
  activeCommand.value = c
  commandsSent.value++
  lastCommandSent.value = c
  addLog('> ' + cmd)
  socket.emit('command', { action: cmd })
  lastOledCommand.value = c

  if (c === 'LIST')  { oledMode.value = 'LIST';  return }
  if (c === 'ALL')   { oledMode.value = 'ALL';   return }
  if (c === 'TEMP'  || c === 'TEMPERATURE') { oledMode.value = 'TEMP';  return }
  if (c === 'PRESS' || c === 'PRESSION')    { oledMode.value = 'PRESS'; return }

  if (c === 'STATUS') {
    oledMode.value       = 'RESPONSE'
    oledResponse1.value  = '=== STATUS ==='
    oledResponse2.value  = fluxValues.value['temperature'] !== undefined ? 'T: OK ' + fluxValues.value['temperature'] + 'C'   : 'T: N/A'
    oledResponse3.value  = fluxValues.value['pression']    !== undefined ? 'P: OK ' + fluxValues.value['pression']    + 'hPa' : 'P: N/A'
    oledResponseOk.value = true
    setTimeout(() => { oledMode.value = 'ALL' }, 3000)
    return
  }

  if (c === 'DESCRIBE') {
    oledMode.value       = 'RESPONSE'
    oledResponse1.value  = 'DESCRIBE'
    oledResponse2.value  = 'Rafraichissement...'
    oledResponse3.value  = 'flux en cours'
    oledResponseOk.value = true
    setTimeout(() => { oledMode.value = 'ALL' }, 2000)
    return
  }

  if (c.endsWith('?')) {
    const nom    = c.replace('?', '').toLowerCase()
    const existe = fluxValues.value[nom] !== undefined
    oledMode.value       = 'RESPONSE'
    oledResponse1.value  = 'Flux ' + nom
    oledResponse2.value  = existe ? '>>> OUI <<<' : '>>> NON <<<'
    oledResponse3.value  = existe ? 'Disponible OK' : 'Flux introuvable'
    oledResponseOk.value = existe
    setTimeout(() => { oledMode.value = 'ALL' }, 3000)
    return
  }

  const fluxId = c.toLowerCase()
  if (fluxValues.value[fluxId] !== undefined) { oledMode.value = c; return }

  oledMode.value       = 'RESPONSE'
  oledResponse1.value  = 'Flux introuvable'
  oledResponse2.value  = c
  oledResponse3.value  = '>>> NON <<<'
  oledResponseOk.value = false
  setTimeout(() => { oledMode.value = 'ALL' }, 3000)
}

function sendOledMessage(message) {
  if (isOffline.value || !socket.connected) { addLog('⚠️ Impossible : application offline'); return }
  if (message === '') { addLog('🗑️ Message OLED effacé'); socket.emit('command', { action: 'MSG:CLEAR' }) }
  else                { addLog('📺 Message OLED: ' + message); socket.emit('command', { action: 'MSG:' + message }) }
}

function clearConsole() { logs.value = [] }
</script>

<style scoped>
.dashboard {
  max-width: 1700px;
  margin: 0 auto;
  padding: 8px 12px;
  display: flex;
  flex-direction: column;
  gap: 8px;
  height: 100vh;
  overflow: hidden;
  box-sizing: border-box;
}

/* ===== HEADER ===== */
.dash-header {
  background: linear-gradient(135deg, var(--accent2) 0%, #1e3a5f 100%);
  border-radius: 10px;
  padding: 10px 20px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  flex-shrink: 0;
}

.header-left  { display: flex; align-items: center; gap: 14px; }
.header-right { display: flex; align-items: center; gap: 10px; }

.header-logo {
  color: #fff; font-size: 18px; font-weight: 600;
  display: flex; align-items: center; gap: 10px;
}

.header-pulse {
  width: 9px; height: 9px; border-radius: 50%;
  background: #4ade80; animation: pulse-anim 1.5s infinite;
}

@keyframes pulse-anim {
  0%, 100% { opacity: 1; transform: scale(1); }
  50%       { opacity: 0.5; transform: scale(0.8); }
}

.header-clock {
  color: rgba(255,255,255,0.9); font-size: 14px;
  font-family: monospace;
  background: rgba(255,255,255,0.12);
  padding: 4px 14px; border-radius: 6px;
  border: 1px solid rgba(255,255,255,0.2);
  letter-spacing: 1px;
}

.badge-live {
  display: flex; align-items: center; gap: 5px;
  background: rgba(74,222,128,0.2); border: 1px solid rgba(74,222,128,0.4);
  color: #4ade80; font-size: 11px; font-weight: 600;
  padding: 3px 10px; border-radius: 20px;
}

.live-dot {
  width: 6px; height: 6px; border-radius: 50%;
  background: #4ade80; animation: pulse-anim 1.5s infinite;
}

.badge-offline {
  background: rgba(248,81,73,0.2); border: 1px solid rgba(248,81,73,0.4);
  color: #f85149; font-size: 11px; font-weight: 700;
  padding: 3px 10px; border-radius: 20px;
}

.theme-toggle {
  background: rgba(255,255,255,0.15); border: 1px solid rgba(255,255,255,0.25);
  color: #fff; font-size: 16px; padding: 5px 12px; border-radius: 8px;
}
.theme-toggle:hover { background: rgba(255,255,255,0.25); }

.hbtn-disconnect {
  background: rgba(255,255,255,0.9); border: none;
  color: var(--accent2); font-size: 13px; font-weight: 600;
  padding: 6px 18px; border-radius: 8px;
}
.hbtn-disconnect:hover { background: #fff; }

/* ===== BARRE STATUT ===== */
.status-bar {
  display: flex; align-items: center; gap: 10px;
  background: var(--card); border: 1px solid var(--border);
  border-radius: 8px; padding: 6px 14px;
  flex-shrink: 0; flex-wrap: wrap;
}

.status-item  { display: flex; align-items: center; gap: 5px; font-size: 12px; white-space: nowrap; }
.status-dot   { width: 7px; height: 7px; border-radius: 50%; flex-shrink: 0; }
.dot-green    { background: var(--green); box-shadow: 0 0 5px var(--green); }
.dot-red      { background: var(--red);   box-shadow: 0 0 5px var(--red); }
.status-label { color: var(--text-muted); font-size: 12px; }
.status-val   { font-weight: 600; font-size: 12px; color: var(--text); }
.val-green    { color: var(--green); }
.val-red      { color: var(--red); }
.val-blue     { color: var(--accent); font-family: monospace; }
.status-sep   { color: var(--border); font-size: 14px; }

.status-backend {
  display: flex; align-items: center; gap: 6px;
  flex: 1; min-width: 200px;
}

.url-label-sm { font-size: 12px; color: var(--text-muted); white-space: nowrap; }

.url-input-sm {
  flex: 1; background: var(--bg3); border: 1px solid var(--border);
  border-radius: 5px; padding: 3px 8px;
  color: var(--text); font-family: monospace; font-size: 12px; outline: none;
}
.url-input-sm:focus { border-color: var(--accent); }

.url-btn-sm {
  padding: 3px 10px; background: rgba(14,165,233,0.1);
  border: 1px solid var(--accent); border-radius: 5px;
  color: var(--accent); font-size: 12px; font-weight: 500; white-space: nowrap;
}
.url-btn-sm:hover { background: rgba(14,165,233,0.2); }

.status-actions { display: flex; gap: 6px; flex-shrink: 0; }

.cmd-action-btn {
  padding: 4px 12px; border-radius: 7px;
  font-size: 12px; font-weight: 500;
  border: 1px solid; background: transparent;
  cursor: pointer; transition: all 0.2s; white-space: nowrap;
}
.help-btn         { border-color: var(--accent); color: var(--accent); }
.help-btn:hover   { background: rgba(14,165,233,0.1); }
.config-btn       { border-color: #a78bfa; color: #a78bfa; }
.config-btn:hover { background: rgba(167,139,250,0.1); }

/* ===== OFFLINE ===== */
.offline-banner {
  background: #ef4444; color: #fff; text-align: center;
  padding: 7px; border-radius: 8px; font-size: 13px; font-weight: 600;
  flex-shrink: 0; animation: blink 1.5s infinite;
}

@keyframes blink {
  0%, 100% { opacity: 1; }
  50%       { opacity: 0.8; }
}

/* ===== LAYOUT ===== */
.main-layout {
  display: grid;
  grid-template-columns: 0.85fr 2.2fr 0.85fr;
  gap: 8px;
  flex: 1;
  min-height: 0;
  overflow: hidden;
}

/* ✅ col — scroll activé */
.col {
  display: flex; flex-direction: column;
  gap: 8px; min-height: 0;
  overflow-y: auto;
  overflow-x: hidden;
}

.col::-webkit-scrollbar       { width: 3px; }
.col::-webkit-scrollbar-track { background: transparent; }
.col::-webkit-scrollbar-thumb { background: var(--border); border-radius: 3px; }

.col > .card-grow { flex: 1; min-height: 0; }

.col-center {
  display: flex; flex-direction: column;
  gap: 8px; min-height: 0; overflow: hidden;
}

/* ===== CARDS ===== */
.card {
  background: var(--card);
  border: 1px solid var(--border);
  border-radius: 10px;
  padding: 10px 12px;
  box-shadow: 0 2px 8px rgba(0,0,0,0.12);
  flex-shrink: 0;
}

.card-grow {
  flex: 1; min-height: 0;
  display: flex; flex-direction: column;
}

.card-header {
  display: flex; align-items: center;
  justify-content: space-between; margin-bottom: 8px;
}

.badge-connected {
  background: rgba(63,185,80,0.15); color: var(--green);
  font-size: 11px; padding: 2px 10px;
  border-radius: 20px; border: 1px solid var(--green);
}
.badge-disconnected {
  background: rgba(248,81,73,0.15); color: var(--red);
  font-size: 11px; padding: 2px 10px;
  border-radius: 20px; border: 1px solid var(--red);
}

/* ===== TITRES ===== */
.section-title {
  font-size: 12px; text-transform: uppercase;
  letter-spacing: 0.8px; color: var(--text-muted);
  display: flex; align-items: center; gap: 6px;
  margin-bottom: 8px; font-weight: 600;
}
.title-dot {
  width: 6px; height: 6px; border-radius: 50%;
  background: var(--accent); flex-shrink: 0;
}

/* ===== DONNÉES IoT ===== */
.data-values { display: flex; gap: 8px; margin-bottom: 6px; }

.data-value-card {
  flex: 1; border-radius: 10px; padding: 10px 8px;
  text-align: center; border: 1px solid; transition: all 0.3s;
}
.data-value-card.disabled { opacity: 0.4; }

.flux-icon { font-size: 18px; margin-bottom: 3px; display: block; }
.data-value-number { font-size: 24px; font-weight: 500; display: block; transition: color 0.5s; }
.data-value-unit   { font-size: 12px; color: var(--text-muted); }
.data-value-label  { font-size: 10px; color: var(--text-hint); display: block; margin-top: 2px; }

.freshness-bar   { margin-bottom: 6px; }
.freshness-label { font-size: 11px; color: var(--text-hint); margin-bottom: 3px; }
.freshness-track { height: 3px; background: var(--bg3); border-radius: 3px; overflow: hidden; }
.freshness-fill  { height: 100%; border-radius: 3px; transition: width 1s linear, background 0.5s; }

/* Stats modernes */
.stats-modern {
  display: flex; flex-direction: column; gap: 6px;
  padding: 8px 10px; background: var(--bg3);
  border-radius: 8px; border: 1px solid var(--border);
}
.stats-modern-row    { display: flex; flex-direction: column; gap: 4px; }
.stats-modern-title  { font-size: 11px; font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px; }
.stats-modern-badges { display: flex; gap: 4px; }

.stat-badge {
  flex: 1; display: flex; flex-direction: column;
  align-items: center; padding: 5px 4px;
  border-radius: 8px; border: 1px solid; gap: 1px;
}
.stat-min { background: rgba(56,189,248,0.08); border-color: rgba(56,189,248,0.2); }
.stat-avg { background: rgba(34,197,94,0.08);  border-color: rgba(34,197,94,0.2); }
.stat-max { background: rgba(239,68,68,0.08);  border-color: rgba(239,68,68,0.2); }
.stat-badge-label { font-size: 9px; color: var(--text-hint); text-transform: uppercase; letter-spacing: 0.5px; font-weight: 600; }
.stat-badge-val   { font-size: 13px; font-weight: 700; }

/* Flux */
.pins { display: flex; gap: 8px; }
.pin-card {
  flex: 1; padding: 8px 6px; border-radius: 10px;
  background: var(--bg3); border: 1px solid var(--border);
  color: var(--text); cursor: pointer; transition: all 0.2s; text-align: center;
}
.pin-card p { font-size: 12px; font-weight: 500; margin-top: 4px; }
.pin-card.active {
  border-color: var(--accent); background: rgba(14,165,233,0.08);
  box-shadow: 0 0 12px rgba(14,165,233,0.2);
}
.flux-desc  { display: block; font-size: 10px; color: var(--text-hint); margin-top: 2px; }
.icon {
  width: 36px; height: 36px; margin: 0 auto 4px;
  border-radius: 9px;
  background: linear-gradient(135deg, var(--accent2), #1e3a5f);
  display: flex; align-items: center; justify-content: center;
  color: #fff; font-weight: 600; font-size: 12px;
}
.detected-info { margin-top: 6px; font-size: 11px; color: var(--text-hint); }
.detected-list { color: var(--green); font-family: monospace; font-weight: 500; }
.waiting-metadata { text-align: center; color: var(--text-hint); font-size: 12px; padding: 8px 0; }

/* État système */
.system-rows { display: flex; flex-direction: column; }
.system-row {
  display: flex; align-items: center; justify-content: space-between;
  padding: 5px 0; border-bottom: 1px solid var(--border);
}
.system-row:last-child { border-bottom: none; }
.sys-label { color: var(--text-muted); font-size: 12px; }
.sys-val {
  display: flex; align-items: center; gap: 6px;
  font-size: 12px; font-weight: 500; color: var(--text);
}
.sys-dot   { width: 6px; height: 6px; border-radius: 50%; flex-shrink: 0; }
.dot-green { background: var(--green); box-shadow: 0 0 4px var(--green); }
.dot-red   { background: var(--red);   box-shadow: 0 0 4px var(--red); }
.sys-green { color: var(--green); }
.sys-red   { color: var(--red); }
.sys-blue  { color: var(--accent); font-family: monospace; font-size: 11px; }

/* Graphique */
.offline-alert { border: 2px solid var(--red) !important; animation: glow 2s infinite; }
@keyframes glow {
  0%, 100% { box-shadow: 0 0 10px rgba(248,81,73,0.3); }
  50%       { box-shadow: 0 0 20px rgba(248,81,73,0.7); }
}

/* ===== COMMANDES ===== */
.cmd-card {
  flex: 1; min-height: 0; max-height: 58%;
  display: flex; flex-direction: column;
}

/* ✅ Scroll commandes */
.cmd-list {
  display: flex; flex-direction: column; gap: 3px;
  overflow-y: auto; flex: 1; min-height: 0; padding-right: 4px;
}
.cmd-list::-webkit-scrollbar       { width: 4px; }
.cmd-list::-webkit-scrollbar-track { background: transparent; }
.cmd-list::-webkit-scrollbar-thumb { background: var(--border); border-radius: 4px; }
.cmd-list::-webkit-scrollbar-thumb:hover { background: var(--accent); }

.cmd-section-label {
  font-size: 10px; font-weight: 600;
  text-transform: uppercase; letter-spacing: 1px;
  padding: 4px 10px; border-radius: 6px;
  margin-top: 5px; margin-bottom: 3px; flex-shrink: 0;
}
.general-label { background: rgba(14,165,233,0.12); color: var(--accent);    border-left: 3px solid var(--accent); }
.flux-label    { background: rgba(52,211,153,0.12); color: var(--secondary); border-left: 3px solid var(--secondary); }
/* ✅ Section personnalisées */
.custom-label  { background: rgba(167,139,250,0.12); color: #a78bfa; border-left: 3px solid #a78bfa; }

.cmd-btn {
  display: flex; align-items: center; gap: 8px;
  width: 100%; padding: 6px 10px;
  border-radius: 8px; border: 1px solid var(--border);
  background: var(--bg3); cursor: pointer;
  transition: all 0.2s; text-align: left; flex-shrink: 0;
}
.cmd-btn:hover  { background: rgba(14,165,233,0.08); border-color: var(--accent); transform: translateX(3px); }
.cmd-btn.active { background: var(--accent); border-color: var(--accent); transform: translateX(3px); box-shadow: 0 0 12px rgba(14,165,233,0.4); }
.cmd-btn.active .cmd-tag  { background: rgba(255,255,255,0.2); color: #fff; border-color: rgba(255,255,255,0.3); }
.cmd-btn.active .cmd-desc { color: #fff; font-weight: 500; }

.cmd-tag {
  font-family: monospace; font-size: 10px;
  padding: 2px 8px; border-radius: 20px;
  min-width: 80px; text-align: center;
  flex-shrink: 0; font-weight: 500; transition: all 0.2s;
}
.general-tag { background: rgba(14,165,233,0.15); color: var(--accent);    border: 1px solid rgba(14,165,233,0.3); }
.flux-tag    { background: rgba(52,211,153,0.15); color: var(--secondary); border: 1px solid rgba(52,211,153,0.3); }
/* ✅ Tag personnalisé violet */
.custom-tag  { background: rgba(167,139,250,0.15); color: #a78bfa; border: 1px solid rgba(167,139,250,0.3); }

.cmd-desc { font-size: 11px; color: var(--text-muted); transition: color 0.2s; }

/* ✅ Console rétractable */
.console-card {
  flex: 1; min-height: 0;
  display: flex; flex-direction: column;
}
.console-card.collapsed { flex: 0; min-height: auto; }

.console-header {
  display: flex; align-items: center;
  justify-content: space-between;
  cursor: pointer; user-select: none;
}
.console-header-actions { display: flex; align-items: center; gap: 8px; }

.console-fullscreen-btn {
  background: none; border: 1px solid var(--border);
  color: var(--text-muted); font-size: 14px;
  padding: 2px 6px; border-radius: 4px; cursor: pointer; transition: all 0.2s;
}
.console-fullscreen-btn:hover { border-color: var(--accent); color: var(--accent); }
.console-toggle { font-size: 12px; color: var(--text-muted); }

.console-body {
  flex: 1; min-height: 0; margin-top: 8px;
  display: flex; flex-direction: column; overflow: hidden;
}

/* ✅ Overlay console plein écran */
.console-overlay {
  position: fixed;
  top: 0; left: 0; right: 0; bottom: 0;
  background: rgba(0,0,0,0.88);
  display: flex; align-items: center; justify-content: center;
  z-index: 200;
}

.console-overlay-panel {
  background: #0d1117;
  border: 1px solid #30363d;
  border-radius: 14px;
  width: 92vw; height: 88vh;
  display: flex; flex-direction: column;
  padding: 20px; gap: 12px;
  box-shadow: 0 0 40px rgba(0,0,0,0.6);
}

.console-overlay-header {
  display: flex; align-items: center;
  justify-content: space-between; flex-shrink: 0;
}

.console-overlay-body {
  flex: 1; min-height: 0;
  display: flex; flex-direction: column;
}

.console-close-btn {
  background: rgba(248,81,73,0.15); border: 1px solid #f85149;
  color: #f85149; font-size: 13px; padding: 5px 14px;
  border-radius: 8px; cursor: pointer; font-weight: 500;
}
.console-close-btn:hover { background: rgba(248,81,73,0.3); }

.value-green { color: var(--green); font-weight: 500; }
.value-red   { color: var(--red);   font-weight: 500; }
</style>