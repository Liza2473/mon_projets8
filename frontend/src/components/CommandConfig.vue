<template>
  <div v-if="isOpen" class="overlay" @click.self="close">
    <div
      class="config-panel"
      :style="{ top: panelY + 'px', left: panelX + 'px' }"
      ref="panelRef"
    >

      <!-- Header déplaçable -->
      <div class="config-header" @mousedown="startDrag">
        <h3>⠿ Configurer les commandes</h3>
        <button class="close-btn" @click="close">✕</button>
      </div>

      <p class="config-subtitle">
        Les commandes sont générées automatiquement depuis les flux de l'appareil.
        Coche celles que tu veux afficher dans le dashboard.
      </p>

      <p class="config-count">{{ selected.length }} commande(s) sélectionnée(s)</p>

      <!-- Aperçu sélection triée alphabétiquement -->
      <div v-if="selected.length" class="selection-preview">
        <span class="preview-label">Sélection actuelle :</span>
        <span class="preview-tags">
          <span v-for="(id, i) in sortedSelected" :key="id" class="preview-tag">
            {{ id }}<span v-if="i < sortedSelected.length - 1"> • </span>
          </span>
        </span>
      </div>

      <!-- Liste avec hauteur fixe + scroll -->
      <div class="config-list">

        <!-- Section 1 : Commandes générales -->
        <div class="config-category">
          <div class="category-header">
            <h4 class="category-title general-title">⚙️ Commandes générales</h4>
            <div class="category-actions">
              <button class="mini-btn" @click="selectAllGeneral">Tout cocher</button>
              <button class="mini-btn" @click="unselectAllGeneral">Tout décocher</button>
            </div>
          </div>

          <label
            v-for="cmd in sortedGeneralSelected"
            :key="cmd.id"
            class="config-item selected-item"
          >
            <input type="checkbox" :value="cmd.id" v-model="selected" />
            <span class="cmd-id general-id">{{ cmd.id }}</span>
            <span class="cmd-label">{{ cmd.label }}</span>
            <span class="selected-badge">✓</span>
          </label>

          <label
            v-for="cmd in sortedGeneralUnselected"
            :key="cmd.id"
            class="config-item"
          >
            <input type="checkbox" :value="cmd.id" v-model="selected" />
            <span class="cmd-id general-id unselected">{{ cmd.id }}</span>
            <span class="cmd-label unselected-label">{{ cmd.label }}</span>
          </label>
        </div>

        <!-- Section 2 : Flux de l'appareil -->
        <div class="config-category">
          <div class="category-header">
            <h4 class="category-title flux-title">📡 Flux de l'appareil</h4>
            <div class="category-actions">
              <button class="mini-btn" @click="selectAllFlux">Tout cocher</button>
              <button class="mini-btn" @click="unselectAllFlux">Tout décocher</button>
            </div>
          </div>

          <div v-if="sortedFluxCommands.length === 0" class="waiting-text">
            ⏳ En attente des flux de l'appareil...
          </div>

          <label
            v-for="cmd in sortedFluxSelected"
            :key="cmd.id"
            class="config-item selected-item"
          >
            <input type="checkbox" :value="cmd.id" v-model="selected" />
            <span class="cmd-id flux-id">{{ cmd.id }}</span>
            <span class="cmd-label">{{ cmd.label }}</span>
            <span class="cmd-desc-small">{{ cmd.desc }}</span>
            <span class="selected-badge">✓</span>
          </label>

          <label
            v-for="cmd in sortedFluxUnselected"
            :key="cmd.id"
            class="config-item"
          >
            <input type="checkbox" :value="cmd.id" v-model="selected" />
            <span class="cmd-id flux-id unselected">{{ cmd.id }}</span>
            <span class="cmd-label unselected-label">{{ cmd.label }}</span>
            <span class="cmd-desc-small">{{ cmd.desc }}</span>
          </label>
        </div>

        <!-- Section 3 : Personnalisées -->
        <div v-if="customCommands.length" class="config-category">
          <div class="category-header">
            <h4 class="category-title custom-title">✏️ Personnalisées</h4>
          </div>
          <label
            v-for="cmd in customCommands"
            :key="cmd"
            class="config-item"
          >
            <input type="checkbox" :value="cmd" v-model="selected" />
            <span class="cmd-id custom-id">{{ cmd }}</span>
            <span class="cmd-label">Commande personnalisée</span>
            <button class="remove-btn" @click.prevent="removeCustom(cmd)">✕</button>
          </label>
        </div>

      </div>

      <!-- Ajouter commande personnalisée -->
      <div class="custom-add">
        <h4 class="category-title custom-title">Ajouter une commande personnalisée</h4>
        <div class="custom-row">
          <input
            v-model="customCmd"
            class="custom-input"
            placeholder="Ex: SOIL_MOISTURE?"
            maxlength="30"
            @keyup.enter="addCustom"
          />
          <button class="add-btn" @click="addCustom">+ Ajouter</button>
        </div>
        <p v-if="customError" class="custom-error">{{ customError }}</p>
      </div>

      <div class="config-footer">
        <button class="reset-btn" @click="resetDefaults">Réinitialiser</button>
        <button class="save-btn" :disabled="!hasChanges" @click="save">
          Enregistrer
        </button>
      </div>

    </div>
  </div>
</template>

<script setup>
import { ref, computed, watch } from 'vue'

const props = defineProps({
  isOpen:       { type: Boolean, default: false },
  fluxMetadata: { type: Array,   default: () => [] }
})

const emit = defineEmits(['close', 'update'])

const selected        = ref([])
const customCommands  = ref([])
const customCmd       = ref('')
const customError     = ref('')
const initialSelected = ref([])
const initialCustom   = ref([])

// ============================================================
// Fenêtre déplaçable
// ============================================================
const panelRef  = ref(null)
const panelX    = ref(0)
const panelY    = ref(0)
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

// Reset position quand on ouvre
watch(() => props.isOpen, (val) => {
  if (val) {
    panelX.value  = 0
    panelY.value  = 0
    loadConfig()
    customError.value = ''
  }
})

// ============================================================
// Commandes générales fixes
// ============================================================
const GENERAL_COMMANDS = [
  { id: 'ALL',      label: 'Affiche tout' },
  { id: 'DESCRIBE', label: 'Rafraîchir les flux' },
  { id: 'LIST',     label: 'Liste les flux' },
  { id: 'STATUS',   label: 'État des flux' },
]

// ============================================================
// Flux depuis props (réactif)
// ============================================================
const fluxMetadata = computed(() => props.fluxMetadata)

const sortedFluxCommands = computed(() => {
  const cmds = []
  fluxMetadata.value.forEach(flux => {
    const id = flux.id.toUpperCase()
    cmds.push({ id, label: flux.label, desc: flux.desc })
    cmds.push({
      id: id + '?',
      label: flux.label + ' existe ?',
      desc: 'Demande si ce capteur est disponible'
    })
  })
  return cmds.sort((a, b) => a.id.localeCompare(b.id))
})

// ============================================================
// Tri : sélectionnés (alpha) en haut, non sélectionnés en bas
// ============================================================
const sortedGeneralSelected = computed(() =>
  GENERAL_COMMANDS
    .filter(c => selected.value.includes(c.id))
    .sort((a, b) => a.id.localeCompare(b.id))
)

const sortedGeneralUnselected = computed(() =>
  GENERAL_COMMANDS
    .filter(c => !selected.value.includes(c.id))
    .sort((a, b) => a.id.localeCompare(b.id))
)

const sortedFluxSelected = computed(() =>
  sortedFluxCommands.value
    .filter(c => selected.value.includes(c.id))
    .sort((a, b) => a.id.localeCompare(b.id))
)

const sortedFluxUnselected = computed(() =>
  sortedFluxCommands.value
    .filter(c => !selected.value.includes(c.id))
    .sort((a, b) => a.id.localeCompare(b.id))
)

const sortedSelected = computed(() =>
  [...selected.value].sort((a, b) => a.localeCompare(b))
)

// ============================================================
// Chargement config
// ============================================================
function loadConfig() {
  const saved       = localStorage.getItem('selected-commands-v2')
  const savedCustom = localStorage.getItem('custom-commands')

  if (saved) {
    selected.value = JSON.parse(saved)
  } else {
    selected.value = [
      ...GENERAL_COMMANDS.map(c => c.id),
      ...sortedFluxCommands.value.map(c => c.id)
    ]
  }

  customCommands.value  = savedCustom ? JSON.parse(savedCustom) : []
  initialSelected.value = [...selected.value]
  initialCustom.value   = [...customCommands.value]
}
loadConfig()

const hasChanges = computed(() => {
  const a = JSON.stringify([...selected.value].sort())
  const b = JSON.stringify([...initialSelected.value].sort())
  const c = JSON.stringify([...customCommands.value].sort())
  const d = JSON.stringify([...initialCustom.value].sort())
  return a !== b || c !== d
})

// ============================================================
// Actions
// ============================================================
function selectAllGeneral() {
  GENERAL_COMMANDS.forEach(c => {
    if (!selected.value.includes(c.id)) selected.value.push(c.id)
  })
}

function unselectAllGeneral() {
  const ids = GENERAL_COMMANDS.map(c => c.id)
  selected.value = selected.value.filter(id => !ids.includes(id))
}

function selectAllFlux() {
  sortedFluxCommands.value.forEach(c => {
    if (!selected.value.includes(c.id)) selected.value.push(c.id)
  })
}

function unselectAllFlux() {
  const ids = sortedFluxCommands.value.map(c => c.id)
  selected.value = selected.value.filter(id => !ids.includes(id))
}

// ✅ Fix 1 — addCustom sauvegarde et émet immédiatement
function addCustom() {
  customError.value = ''
  const val = customCmd.value.trim().toUpperCase()
  if (!val) return

  const allIds = [
    ...GENERAL_COMMANDS.map(c => c.id),
    ...sortedFluxCommands.value.map(c => c.id),
    ...customCommands.value
  ]

  if (allIds.includes(val)) {
    customError.value = 'Cette commande existe déjà.'
    return
  }

  customCommands.value.push(val)
  selected.value.push(val)
  customCmd.value = ''

  // ✅ Sauvegarde + émission immédiate → Dashboard.vue voit le bouton
  localStorage.setItem('selected-commands-v2', JSON.stringify(selected.value))
  localStorage.setItem('custom-commands', JSON.stringify(customCommands.value))
  emit('update', { selected: selected.value, custom: customCommands.value })

  // Sync initialSelected pour ne pas bloquer le bouton Enregistrer
  initialSelected.value = [...selected.value]
  initialCustom.value   = [...customCommands.value]
}

function removeCustom(cmd) {
  customCommands.value = customCommands.value.filter(c => c !== cmd)
  selected.value       = selected.value.filter(c => c !== cmd)
  localStorage.setItem('selected-commands-v2', JSON.stringify(selected.value))
  localStorage.setItem('custom-commands', JSON.stringify(customCommands.value))
  emit('update', { selected: selected.value, custom: customCommands.value })
  initialSelected.value = [...selected.value]
  initialCustom.value   = [...customCommands.value]
}

function resetDefaults() {
  selected.value = [
    ...GENERAL_COMMANDS.map(c => c.id),
    ...sortedFluxCommands.value.map(c => c.id)
  ]
  customCommands.value = []
  customError.value    = ''
}

function close() { emit('close') }

function save() {
  localStorage.setItem('selected-commands-v2', JSON.stringify(selected.value))
  localStorage.setItem('custom-commands',       JSON.stringify(customCommands.value))
  emit('update', { selected: selected.value, custom: customCommands.value })
  close()
}
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

.config-panel {
  background: #1e293b;
  border: 1px solid #334155;
  border-radius: 16px;
  padding: 24px;
  width: 620px;
  max-height: 85vh;
  overflow-y: auto;
  display: flex;
  flex-direction: column;
  gap: 14px;
  position: relative;
}

/* Header draggable */
.config-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  cursor: grab;
  user-select: none;
  padding-bottom: 4px;
  border-bottom: 1px solid #334155;
}

.config-header:active { cursor: grabbing; }

.config-header h3 {
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

.config-subtitle {
  color: #94a3b8;
  font-size: 13px;
  line-height: 1.5;
  margin: 0;
}

.config-count {
  color: #38bdf8;
  font-size: 12px;
  font-weight: 500;
  margin: 0;
}

.selection-preview {
  background: rgba(56,189,248,0.05);
  border: 1px solid rgba(56,189,248,0.15);
  border-radius: 8px;
  padding: 8px 12px;
  font-size: 12px;
  max-height: 80px;
  overflow-y: auto;
}

.preview-label {
  color: #94a3b8;
  display: block;
  margin-bottom: 4px;
}

.preview-tags {
  color: #38bdf8;
  font-family: monospace;
  line-height: 1.6;
}

/* ✅ Fix 2 — scroll visible et stylisé */
.config-list {
  display: flex;
  flex-direction: column;
  gap: 16px;
  max-height: 340px;
  overflow-y: scroll;
  padding-right: 4px;
  scrollbar-width: thin;
  scrollbar-color: #334155 transparent;
}

.config-list::-webkit-scrollbar       { width: 5px; }
.config-list::-webkit-scrollbar-track { background: transparent; }
.config-list::-webkit-scrollbar-thumb {
  background: #334155;
  border-radius: 5px;
}
.config-list::-webkit-scrollbar-thumb:hover { background: #475569; }

.config-category {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.category-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 6px;
}

.category-title {
  font-size: 12px;
  text-transform: uppercase;
  margin: 0;
  letter-spacing: 1px;
}

.general-title { color: #38bdf8; }
.flux-title    { color: #22c55e; }
.custom-title  { color: #a78bfa; }

.category-actions { display: flex; gap: 6px; }

.mini-btn {
  background: none;
  border: 1px solid #334155;
  border-radius: 4px;
  color: #94a3b8;
  font-size: 10px;
  padding: 2px 6px;
  cursor: pointer;
  transition: all 0.15s;
}

.mini-btn:hover {
  border-color: #38bdf8;
  color: #38bdf8;
}

.config-item {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 6px 8px;
  border-radius: 6px;
  cursor: pointer;
  transition: background 0.15s;
}

.config-item:hover { background: rgba(255,255,255,0.04); }

.selected-item {
  background: rgba(56,189,248,0.04);
  border-left: 2px solid rgba(56,189,248,0.3);
}

.config-item input {
  accent-color: #38bdf8;
  width: 16px;
  height: 16px;
  cursor: pointer;
  flex-shrink: 0;
}

.cmd-id {
  font-family: monospace;
  font-size: 12px;
  padding: 2px 8px;
  border-radius: 4px;
  min-width: 100px;
  text-align: center;
  flex-shrink: 0;
}

.general-id  { color: #38bdf8; background: rgba(56,189,248,0.1); }
.flux-id     { color: #22c55e; background: rgba(34,197,94,0.1);  }
.custom-id   { color: #a78bfa; background: rgba(167,139,250,0.1);}

.unselected       { opacity: 0.5; }
.unselected-label { color: #64748b; }

.cmd-label      { font-size: 13px; color: #f1f5f9; flex: 1; }
.cmd-desc-small { font-size: 11px; color: #64748b; }

.selected-badge {
  color: #22c55e;
  font-size: 12px;
  font-weight: bold;
  margin-left: auto;
  flex-shrink: 0;
}

.waiting-text {
  color: #64748b;
  font-size: 13px;
  padding: 10px 0;
  font-style: italic;
}

.custom-add {
  border-top: 1px solid #334155;
  padding-top: 14px;
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.custom-row { display: flex; gap: 8px; }

.custom-input {
  flex: 1;
  background: rgba(255,255,255,0.05);
  border: 1px solid #334155;
  border-radius: 8px;
  padding: 8px 12px;
  color: white;
  font-family: monospace;
  font-size: 13px;
  outline: none;
}

.custom-input:focus { border-color: #a78bfa; }

.add-btn {
  padding: 8px 14px;
  background: rgba(167,139,250,0.15);
  border: 1px solid #a78bfa;
  border-radius: 8px;
  color: #a78bfa;
  font-size: 13px;
  cursor: pointer;
  white-space: nowrap;
}

.add-btn:hover { background: rgba(167,139,250,0.25); }

.custom-error {
  color: #ef4444;
  font-size: 12px;
  margin: 0;
}

.remove-btn {
  background: none;
  border: none;
  color: #ef4444;
  cursor: pointer;
  font-size: 12px;
  padding: 0;
  margin-left: auto;
}

.config-footer {
  display: flex;
  justify-content: space-between;
  align-items: center;
  border-top: 1px solid #334155;
  padding-top: 14px;
}

.reset-btn {
  padding: 10px 18px;
  background: rgba(255,255,255,0.03);
  border: 1px solid #334155;
  border-radius: 8px;
  color: #94a3b8;
  font-size: 13px;
  cursor: pointer;
}

.reset-btn:hover {
  border-color: #94a3b8;
  color: #f1f5f9;
}

.save-btn {
  padding: 10px 24px;
  background: rgba(34,197,94,0.15);
  border: 1px solid #22c55e;
  border-radius: 8px;
  color: #22c55e;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
}

.save-btn:hover:not(:disabled) { background: rgba(34,197,94,0.25); }

.save-btn:disabled {
  opacity: 0.4;
  cursor: not-allowed;
  background: rgba(255,255,255,0.03);
  border-color: #334155;
  color: #94a3b8;
}
</style>