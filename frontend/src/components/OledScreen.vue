<template>
  <div class="oled-wrapper">
    <p class="oled-label">Simulation écran OLED 128x64</p>

    <div class="oled-outer" :class="{ flash: isFlashing }">
      <div class="scanline"></div>

      <div v-if="customDisplayMessage" class="message-overflow-row">
        <span class="msg-overflow">{{ customDisplayMessage }}</span>
      </div>

      <div class="oled-screen">
        <div class="pixel-overlay"></div>
        <div class="oled-inner">

          <template v-if="mode === 'ALL'">
            <span class="ol title">=== ALL FLUX ===</span>
            <span class="ol" :class="tempOk ? 'ok' : 'na'">{{ tempOk ? 'T: ' + temperature + ' C' : 'T: N/A' }}</span>
            <span class="ol" :class="presOk ? 'ok' : 'na'">{{ presOk ? 'P: ' + pression + ' hPa' : 'P: N/A' }}</span>
            <span class="ol sep">----------------</span>
            <span class="ol ok">MQTT: OK</span>
            <span v-if="customDisplayMessage" class="ol msg">{{ truncate(customDisplayMessage) }}</span>
            <span v-else class="ol ok blink">█</span>
          </template>

          <template v-else-if="mode === 'TEMP'">
            <span class="ol title">Flux: Temperature</span>
            <span class="ol sep">----------------</span>
            <span class="ol" :class="tempOk ? 'ok' : 'na'">{{ tempOk ? 'Val: ' + temperature + ' C' : 'Status: N/A' }}</span>
            <span class="ol" :class="tempOk ? 'ok' : 'na'">{{ tempOk ? 'Status: OK' : 'Hors plage' }}</span>
            <span v-if="customDisplayMessage" class="ol msg">{{ truncate(customDisplayMessage) }}</span>
            <span v-else class="ol ok blink">█</span>
          </template>

          <template v-else-if="mode === 'PRESS'">
            <span class="ol title">Flux: Pression</span>
            <span class="ol sep">----------------</span>
            <span class="ol" :class="presOk ? 'ok' : 'na'">{{ presOk ? 'Val: ' + pression + ' hPa' : 'Status: N/A' }}</span>
            <span class="ol" :class="presOk ? 'ok' : 'na'">{{ presOk ? 'Status: OK' : 'Hors plage' }}</span>
            <span v-if="customDisplayMessage" class="ol msg">{{ truncate(customDisplayMessage) }}</span>
            <span v-else class="ol ok blink">█</span>
          </template>

          <template v-else-if="mode === 'LIST'">
            <span class="ol title">Flux disponibles</span>
            <span class="ol sep">----------------</span>
            <span class="ol" :class="tempOk ? 'ok' : 'na'">{{ tempOk ? '+ temperature OK' : '- temperature N/A' }}</span>
            <span class="ol" :class="presOk ? 'ok' : 'na'">{{ presOk ? '+ pression OK' : '- pression N/A' }}</span>
            <span class="ol sep">----------------</span>
            <span v-if="customDisplayMessage" class="ol msg">{{ truncate(customDisplayMessage) }}</span>
            <span v-else class="ol ok">Total: 2 flux</span>
          </template>

          <template v-else-if="mode === 'RESPONSE'">
            <span class="ol title">{{ responseLine1 }}</span>
            <span class="ol" :class="responseOk ? 'ok' : 'na'">{{ responseLine2 }}</span>
            <span class="ol" :class="responseOk ? 'ok' : 'na'">{{ responseLine3 }}</span>
            <span v-if="customDisplayMessage" class="ol msg">{{ truncate(customDisplayMessage) }}</span>
            <span v-else class="ol ok blink">█</span>
          </template>

        </div>
      </div>
    </div>

    <!-- ✅ Historique dernière commande -->
    <div class="oled-history">
      <span class="history-label">Dernière commande :</span>
      <span class="history-value">{{ lastCommand || '—' }}</span>
    </div>

    <!-- Légende -->
    <div class="oled-legend">
      <span class="leg-item"><span class="leg-box visible"></span> Affiché sur l'écran</span>
      <span class="leg-item"><span class="leg-box overflow"></span> Hors écran (débordement)</span>
    </div>

    <!-- Champ message -->
    <div class="oled-input-row">
      <input
        v-model="customMessage"
        class="oled-input"
        placeholder="Tape un message pour la dernière ligne..."
        maxlength="200"
        @keyup.enter="sendCustomMessage"
      />
      <button class="oled-send-btn" @click="sendCustomMessage">Envoyer</button>
      <button class="oled-clear-btn" @click="clearMessage">Effacer</button>
    </div>

    <p class="oled-info">
      Petit cadre = écran réel (21 chars × 6 lignes) — Grand cadre = débordement
    </p>
  </div>
</template>

<script setup>
import { computed, ref, watch } from 'vue'

const props = defineProps({
  mode:          { type: String,           default: 'ALL' },
  temperature:   { type: [Number, String], default: '--' },
  pression:      { type: [Number, String], default: '--' },
  responseLine1: { type: String,           default: '' },
  responseLine2: { type: String,           default: '' },
  responseLine3: { type: String,           default: '' },
  responseOk:    { type: Boolean,          default: false },
  lastCommand:   { type: String,           default: '' }
})

const emit = defineEmits(['send-message'])

const customMessage        = ref('')
const customDisplayMessage = ref('')
const isFlashing           = ref(false)

const tempOk = computed(() => {
  const v = Number(props.temperature)
  return !isNaN(v) && v > -40 && v < 85
})

const presOk = computed(() => {
  const v = Number(props.pression)
  return !isNaN(v) && v > 300 && v < 1200
})

// ✅ Flash du contour quand le mode change
watch(() => props.mode, () => {
  isFlashing.value = true
  setTimeout(() => { isFlashing.value = false }, 400)
})

function truncate(text) {
  if (!text) return ''
  return text.length > 21 ? text.substring(0, 19) + '..' : text
}

function sendCustomMessage() {
  if (!customMessage.value.trim()) return
  customDisplayMessage.value = customMessage.value.trim()
  emit('send-message', customDisplayMessage.value)
  customMessage.value = ''
}

function clearMessage() {
  customDisplayMessage.value = ''
  customMessage.value = ''
  emit('send-message', '')
}
</script>

<style scoped>
.oled-wrapper {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 10px;
}

.oled-label {
  font-size: 11px;
  color: var(--text-muted);
  text-transform: uppercase;
  letter-spacing: 1px;
}

.oled-outer {
  position: relative;
  width: 420px;
  height: 200px;
  background: #050505;
  border: 1px solid #222;
  border-radius: 12px;
  overflow: hidden;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: border-color 0.1s;
}

/* ✅ Animation contour vert */
.oled-outer.flash {
  animation: flash-border 0.4s ease-out;
}

@keyframes flash-border {
  0%   { border-color: #00ff88; box-shadow: 0 0 20px rgba(0,255,128,0.6); }
  100% { border-color: #222;    box-shadow: none; }
}

.message-overflow-row {
  position: absolute;
  bottom: 22px;
  left: 0;
  right: 0;
  display: flex;
  justify-content: center;
  pointer-events: none;
  z-index: 1;
}

.msg-overflow {
  font-family: 'Courier New', monospace;
  font-size: 11px;
  color: #0d2a0d;
  white-space: nowrap;
}

/* ✅ Petit cadre centré */
.oled-screen {
  position: relative;
  width: 200px;
  height: 116px;
  background: #000;
  border: 1.5px solid #00ff8866;
  border-radius: 4px;
  padding: 6px 7px;
  overflow: hidden;
  box-shadow:
    0 0 10px rgba(0, 255, 128, 0.2),
    inset 0 0 6px rgba(0,0,0,0.9);
  z-index: 2;
  flex-shrink: 0;
}

/* ✅ Effet pixel OLED */
.pixel-overlay {
  position: absolute;
  top: 0; left: 0; right: 0; bottom: 0;
  background: repeating-linear-gradient(
    0deg,
    transparent,
    transparent 2px,
    rgba(0,0,0,0.15) 2px,
    rgba(0,0,0,0.15) 3px
  );
  pointer-events: none;
  z-index: 5;
}

.oled-inner {
  position: relative;
  z-index: 6;
  display: flex;
  flex-direction: column;
  gap: 1px;
}

.ol {
  font-family: 'Courier New', monospace;
  font-size: 11px;
  line-height: 1.4;
  white-space: nowrap;
  overflow: hidden;
  max-width: 21ch;
}

.ol.title { color: #ccffcc; font-weight: bold; }
.ol.sep   { color: #1a3a1a; }
.ol.ok    { color: #00cc66; }
.ol.na    { color: #cc3333; }
.ol.msg   { color: #ccaa00; }

.blink { animation: blink 1s step-start infinite; }

@keyframes blink {
  0%, 100% { opacity: 1; }
  50%       { opacity: 0; }
}

.scanline {
  position: absolute;
  top: 0; left: 0; right: 0;
  height: 2px;
  background: rgba(0, 255, 128, 0.08);
  animation: scan 4s linear infinite;
  pointer-events: none;
  z-index: 10;
}

@keyframes scan {
  0%   { top: 0; }
  100% { top: 100%; }
}

/* ✅ Historique */
.oled-history {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 12px;
}

.history-label { color: var(--text-muted); }

.history-value {
  color: #00ff88;
  font-family: 'Courier New', monospace;
  font-weight: bold;
  background: rgba(0, 255, 128, 0.08);
  padding: 2px 8px;
  border-radius: 4px;
  border: 1px solid #00ff8833;
}

.oled-legend {
  display: flex;
  gap: 20px;
  font-size: 11px;
  color: var(--text-muted);
}

.leg-item {
  display: flex;
  align-items: center;
  gap: 5px;
}

.leg-box {
  width: 12px;
  height: 12px;
  border-radius: 2px;
}

.leg-box.visible  { background: #00ff8822; border: 1px solid #00ff88; }
.leg-box.overflow { background: #0d2a0d;   border: 1px solid #1a3a1a; }

.oled-input-row {
  display: flex;
  gap: 8px;
  width: 420px;
}

.oled-input {
  flex: 1;
  background: rgba(255,255,255,0.05);
  border: 1px solid var(--border-soft);
  border-radius: 8px;
  padding: 8px 12px;
  color: white;
  font-family: 'Courier New', monospace;
  font-size: 13px;
  outline: none;
  transition: border-color 0.2s;
}

.oled-input:focus {
  border-color: #00ff88;
  box-shadow: 0 0 8px rgba(0, 255, 128, 0.2);
}

.oled-send-btn {
  padding: 8px 14px;
  background: rgba(0, 255, 128, 0.15);
  border: 1px solid #00ff88;
  border-radius: 8px;
  color: #00ff88;
  font-size: 13px;
  cursor: pointer;
  transition: all 0.2s;
  white-space: nowrap;
}

.oled-send-btn:hover {
  background: rgba(0, 255, 128, 0.3);
  box-shadow: 0 0 12px rgba(0, 255, 128, 0.3);
}

.oled-clear-btn {
  padding: 8px 14px;
  background: rgba(239, 68, 68, 0.15);
  border: 1px solid #ef4444;
  border-radius: 8px;
  color: #ef4444;
  font-size: 13px;
  cursor: pointer;
  transition: all 0.2s;
  white-space: nowrap;
}

.oled-clear-btn:hover {
  background: rgba(239, 68, 68, 0.3);
  box-shadow: 0 0 12px rgba(239, 68, 68, 0.3);
}

.oled-info {
  font-size: 11px;
  color: var(--text-muted);
  text-align: center;
}
</style>