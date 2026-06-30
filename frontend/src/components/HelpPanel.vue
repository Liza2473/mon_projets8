<template>
  <div v-if="isOpen" class="overlay" @click.self="close">
    <div
      class="help-panel"
      :style="{ top: panelY + 'px', left: panelX + 'px' }"
    >

      <!-- ✅ Header déplaçable -->
      <div class="help-header" @mousedown="startDrag">
        <h3>⠿ Aide — Documentation des commandes</h3>
        <button class="close-btn" @click="close">✕</button>
      </div>

      <p class="help-subtitle">
        Les commandes et descriptions sont générées automatiquement
        depuis les flux publiés par l'appareil connecté.
      </p>

      <div class="help-toolbar">
        <input
          v-model="searchQuery"
          class="search-input"
          placeholder="🔍 Rechercher une commande..."
        />
        <span class="cmd-count">{{ allFilteredCommands.length }} commande(s)</span>
      </div>

      <!-- Comment ça marche -->
      <div class="help-section">
        <h4 class="section-title">Comment ça marche ?</h4>
        <ul class="help-list">
          <li>Le frontend envoie la commande à l'appareil via MQTT</li>
          <li>L'appareil vérifie s'il possède le capteur demandé</li>
          <li>Si oui → il répond avec la valeur mesurée</li>
          <li>Si non → il répond "Flux introuvable / NON"</li>
          <li><strong>Frontend :</strong> s'adapte automatiquement à tout appareil</li>
          <li><strong>Appareil :</strong> déclare ses flux avec leurs descriptions</li>
        </ul>
      </div>

      <!-- ✅ Section générales (bleu) -->
      <div class="help-section">
        <h4 class="section-title general-title">⚙️ Commandes générales</h4>
        <div v-if="filteredGeneral.length === 0" class="no-results">Aucun résultat</div>
        <div v-for="cmd in filteredGeneral" :key="cmd.id" class="help-item">
          <span class="cmd-id general-id">{{ cmd.id }}</span>
          <div class="help-text">
            <span class="cmd-label">{{ cmd.label }}</span>
            <span class="cmd-desc">{{ cmd.desc }}</span>
          </div>
        </div>
      </div>

      <!-- ✅ Section flux Arduino (vert) -->
      <div class="help-section">
        <h4 class="section-title flux-title">📡 Flux de l'appareil</h4>

        <div v-if="fluxMetadata.length === 0" class="waiting-text">
          ⏳ En attente des flux de l'appareil...
        </div>

        <div v-if="filteredFlux.length === 0 && fluxMetadata.length > 0" class="no-results">
          Aucun résultat pour "{{ searchQuery }}"
        </div>

        <div v-for="cmd in filteredFlux" :key="cmd.id" class="help-item">
          <span class="cmd-id flux-id">{{ cmd.id }}</span>
          <div class="help-text">
            <span class="cmd-label">{{ cmd.label }}</span>
            <span class="cmd-desc">{{ cmd.desc }}</span>
          </div>
        </div>
      </div>

      <!-- Architecture -->
      <div class="help-section">
        <h4 class="section-title">Architecture du système</h4>
        <div class="architecture-diagram">
          <div class="arch-box arch-frontend">
            <span class="arch-title">Frontend</span>
            <span class="arch-sub">Vue.js + PWA — s'adapte à tout appareil</span>
          </div>
          <div class="arch-arrow">↓ WebSocket (Socket.IO)</div>
          <div class="arch-box arch-backend">
            <span class="arch-title">Backend</span>
            <span class="arch-sub">Node.js — relaie MQTT ↔ WebSocket</span>
          </div>
          <div class="arch-arrow">↓ MQTT (publish / subscribe)</div>
          <div class="arch-box arch-broker">
            <span class="arch-title">Broker MQTT</span>
            <span class="arch-sub">HiveMQ — diffusion publique</span>
          </div>
          <div class="arch-arrow">↓ MQTT (publish / subscribe)</div>
          <div class="arch-box arch-device">
            <span class="arch-title">Microcontrôleur</span>
            <span class="arch-sub">{{ deviceId || 'ESP8266' }} — déclare ses flux + mesure</span>
          </div>
        </div>
      </div>

      <div class="help-footer">
        <button class="close-btn-2" @click="close">Fermer</button>
      </div>

    </div>
  </div>
</template>

<script setup>
import { ref, computed, watch } from 'vue'

const props = defineProps({
  isOpen:       { type: Boolean, default: false },
  fluxMetadata: { type: Array,   default: () => [] },
  deviceId:     { type: String,  default: '' }
})

const emit = defineEmits(['close'])

const searchQuery = ref('')

// ============================================================
// ✅ Fenêtre déplaçable
// ============================================================
const panelX = ref(0)
const panelY = ref(0)
let isDragging  = false
let startMouseX = 0
let startMouseY = 0
let startPanelX = 0
let startPanelY = 0

function startDrag(e) {
  if (e.target.classList.contains('close-btn')) return
  isDragging  = true
  startMouseX = e.clientX
  startMouseY = e.clientY
  startPanelX = panelX.value
  startPanelY = panelY.value
  window.addEventListener('mousemove', onDrag)
  window.addEventListener('mouseup',   stopDrag)
}

function onDrag(e) {
  if (!isDragging) return
  panelX.value = startPanelX + (e.clientX - startMouseX)
  panelY.value = startPanelY + (e.clientY - startMouseY)
}

function stopDrag() {
  isDragging = false
  window.removeEventListener('mousemove', onDrag)
  window.removeEventListener('mouseup',   stopDrag)
}

// Reset position à chaque ouverture
watch(() => props.isOpen, (val) => {
  if (val) {
    panelX.value  = 0
    panelY.value  = 0
    searchQuery.value = ''
  }
})

// ============================================================
// Commandes générales fixes
// ============================================================
const GENERAL_COMMANDS = [
  { id: 'ALL',      label: 'Affiche tout',        desc: 'Affiche toutes les valeurs disponibles en une fois' },
  { id: 'DESCRIBE', label: 'Rafraîchir les flux', desc: 'Demande à l\'appareil de republier ses flux disponibles' },
  { id: 'LIST',     label: 'Liste les flux',       desc: 'Affiche la liste de tous les flux disponibles sur l\'appareil' },
  { id: 'STATUS',   label: 'État des flux',        desc: 'Affiche l\'état (OK / N/A) de chaque flux disponible' },
]

// ============================================================
// Commandes flux depuis props (réactif)
// ============================================================
const fluxCommands = computed(() => {
  const cmds = []
  props.fluxMetadata.forEach(flux => {
    const id = flux.id.toUpperCase()
    cmds.push({
      id,
      label: flux.label,
      desc: flux.desc + (flux.unit ? ` (unité : ${flux.unit})` : '')
    })
    cmds.push({
      id: id + '?',
      label: flux.label + ' existe ?',
      desc: `Demande à l'appareil si le capteur "${flux.id}" est disponible. Répond OUI ou NON.`
    })
  })
  return cmds.sort((a, b) => a.id.localeCompare(b.id))
})

// ============================================================
// Filtres par recherche
// ============================================================
const filteredGeneral = computed(() => {
  const sorted = [...GENERAL_COMMANDS].sort((a, b) => a.id.localeCompare(b.id))
  if (!searchQuery.value.trim()) return sorted
  const q = searchQuery.value.toLowerCase()
  return sorted.filter(c =>
    c.id.toLowerCase().includes(q) ||
    c.label.toLowerCase().includes(q) ||
    c.desc.toLowerCase().includes(q)
  )
})

const filteredFlux = computed(() => {
  if (!searchQuery.value.trim()) return fluxCommands.value
  const q = searchQuery.value.toLowerCase()
  return fluxCommands.value.filter(c =>
    c.id.toLowerCase().includes(q) ||
    c.label.toLowerCase().includes(q) ||
    c.desc.toLowerCase().includes(q)
  )
})

const allFilteredCommands = computed(() =>
  [...filteredGeneral.value, ...filteredFlux.value]
)

function close() { emit('close') }
</script>

<style scoped>
.overlay {
  position: fixed;
  top: 0; left: 0; right: 0; bottom: 0;
  background: rgba(0,0,0,0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 100;
}

.help-panel {
  background: #1e293b;
  border: 1px solid #334155;
  border-radius: 16px;
  padding: 24px;
  width: 800px;
  max-height: 85vh;
  overflow-y: auto;
  display: flex;
  flex-direction: column;
  gap: 16px;
  position: relative;
}

/* ✅ Header draggable */
.help-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  cursor: grab;
  user-select: none;
  padding-bottom: 8px;
  border-bottom: 1px solid #334155;
}

.help-header:active { cursor: grabbing; }

.help-header h3 {
  color: #f1f5f9;
  font-size: 18px;
  margin: 0;
}

.close-btn {
  background: none;
  border: none;
  color: #94a3b8;
  font-size: 18px;
  cursor: pointer;
}

.help-subtitle {
  color: #94a3b8;
  font-size: 13px;
  line-height: 1.5;
  margin: 0;
}

.help-toolbar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 12px;
}

.search-input {
  flex: 1;
  background: rgba(255,255,255,0.05);
  border: 1px solid #334155;
  border-radius: 8px;
  padding: 8px 12px;
  color: white;
  font-size: 13px;
  outline: none;
}

.search-input:focus { border-color: #38bdf8; }

.cmd-count {
  color: #38bdf8;
  font-size: 12px;
  font-weight: 500;
  white-space: nowrap;
}

.help-section {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.section-title {
  font-size: 13px;
  text-transform: uppercase;
  letter-spacing: 1px;
  margin: 0;
  color: #38bdf8;
}

.general-title { color: #38bdf8; }
.flux-title    { color: #22c55e; }

.help-list {
  margin: 0;
  padding-left: 18px;
  color: #cbd5e1;
  font-size: 13px;
  line-height: 1.8;
}

.help-item {
  display: flex;
  align-items: flex-start;
  gap: 12px;
  padding: 8px 0;
  border-bottom: 1px solid rgba(255,255,255,0.04);
}

.cmd-id {
  font-family: monospace;
  font-size: 12px;
  padding: 2px 8px;
  border-radius: 4px;
  min-width: 110px;
  text-align: center;
  flex-shrink: 0;
  margin-top: 2px;
}

.general-id { color: #38bdf8; background: rgba(56,189,248,0.1); }
.flux-id    { color: #22c55e; background: rgba(34,197,94,0.1);  }

.help-text {
  display: flex;
  flex-direction: column;
  gap: 2px;
}

.cmd-label { font-size: 13px; color: #f1f5f9; font-weight: 500; }
.cmd-desc  { font-size: 12px; color: #94a3b8; }

.no-results {
  color: #94a3b8;
  font-size: 13px;
  padding: 10px 0;
  font-style: italic;
}

.waiting-text {
  color: #64748b;
  font-size: 13px;
  padding: 10px 0;
  font-style: italic;
}

.architecture-diagram {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 4px;
  padding: 10px 0;
}

.arch-box {
  width: 340px;
  padding: 12px 20px;
  border-radius: 10px;
  text-align: center;
  display: flex;
  flex-direction: column;
  gap: 2px;
}

.arch-title { font-size: 14px; font-weight: 600; }
.arch-sub   { font-size: 11px; opacity: 0.7; }

.arch-frontend { background: rgba(167,139,250,0.12); border: 1px solid #a78bfa; color: #a78bfa; }
.arch-backend  { background: rgba(56,189,248,0.12);  border: 1px solid #38bdf8; color: #38bdf8; }
.arch-broker   { background: rgba(245,158,11,0.12);  border: 1px solid #f59e0b; color: #f59e0b; }
.arch-device   { background: rgba(34,197,94,0.12);   border: 1px solid #22c55e; color: #22c55e; }

.arch-arrow {
  color: #64748b;
  font-size: 12px;
  font-family: monospace;
}

.help-footer {
  display: flex;
  justify-content: flex-end;
  border-top: 1px solid #334155;
  padding-top: 14px;
}

.close-btn-2 {
  padding: 10px 24px;
  background: rgba(56,189,248,0.15);
  border: 1px solid #38bdf8;
  border-radius: 8px;
  color: #38bdf8;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
}

.close-btn-2:hover { background: rgba(56,189,248,0.3); }
</style>