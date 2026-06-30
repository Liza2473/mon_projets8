<template>
  <div class="console-wrapper">

    <div class="console-box" ref="consoleBox">
      <div v-if="logs.length === 0" class="console-empty">Console vide</div>
      <div
        v-for="(l, i) in logs"
        :key="i"
        class="console-line"
        :class="{
          'log-green':  l.startsWith('✅') || l.startsWith('📡') || l.startsWith('📋'),
          'log-blue':   l.startsWith('📤') || l.startsWith('🔄') || l.startsWith('📺'),
          'log-red':    l.startsWith('❌') || l.startsWith('⚠️'),
          'log-purple': l.startsWith('⚙️') || l.startsWith('🔌'),
          'log-cmd':    l.startsWith('>'),
        }"
      >{{ l }}</div>
    </div>

    <div class="input-row">
      <input
        v-model="cmd"
        class="console-input"
        placeholder="Entrer une commande..."
        @keyup.enter="send"
      />
      <button class="clear-btn" @click="clear" title="Effacer">🗑</button>
    </div>

  </div>
</template>

<script setup>
import { ref, watch, nextTick } from 'vue'

const props = defineProps({
  logs: { type: Array, default: () => [] }
})

const emit = defineEmits(['send-command', 'clear-console'])
const cmd        = ref('')
const consoleBox = ref(null)

// ✅ Auto-scroll vers le bas à chaque nouveau log
watch(() => props.logs.length, async () => {
  await nextTick()
  if (consoleBox.value) {
    consoleBox.value.scrollTop = consoleBox.value.scrollHeight
  }
})

function send() {
  if (!cmd.value.trim()) return
  emit('send-command', cmd.value.trim())
  cmd.value = ''
}

function clear() {
  emit('clear-console')
}
</script>

<style scoped>
.console-wrapper {
  display: flex;
  flex-direction: column;
  gap: 8px;
  height: 100%;
  min-height: 0;
}

/* ✅ Console — grandit pour remplir l'espace */
.console-box {
  flex: 1;
  min-height: 0;
  background: #0d1117;
  border: 1px solid #30363d;
  border-radius: 10px;
  overflow-y: auto;
  padding: 10px 12px;
  font-family: 'Consolas', 'Courier New', monospace;
  font-size: 12px;
  line-height: 1.7;
  scrollbar-width: thin;
  scrollbar-color: #30363d transparent;
}

.console-box::-webkit-scrollbar       { width: 4px; }
.console-box::-webkit-scrollbar-track { background: transparent; }
.console-box::-webkit-scrollbar-thumb {
  background: #30363d;
  border-radius: 4px;
}

.console-empty {
  color: #6e7681;
  font-style: italic;
}

/* ✅ Couleurs des logs */
.console-line  { color: #c9d1d9; }
.log-green     { color: #3fb950; }
.log-blue      { color: #58a6ff; }
.log-red       { color: #f85149; }
.log-purple    { color: #d2a8ff; }
.log-cmd       { color: #e3b341; font-weight: 500; }

/* ✅ Input */
.input-row {
  display: flex;
  gap: 8px;
  flex-shrink: 0;
}

.console-input {
  flex: 1;
  background: var(--bg3);
  border: 1px solid var(--border);
  border-radius: 8px;
  padding: 6px 12px;
  color: var(--text);
  font-size: 13px;
  font-family: 'Consolas', monospace;
  outline: none;
  transition: border-color 0.2s;
}

.console-input:focus { border-color: var(--accent); }

[data-theme="light"] .console-input {
  background: #f8fafc;
  border-color: #94a3b8;
  color: #0f172a;
}

[data-theme="light"] .console-input::placeholder { color: #64748b; }

.clear-btn {
  background: var(--bg3);
  border: 1px solid var(--border);
  border-radius: 8px;
  padding: 6px 12px;
  font-size: 14px;
  color: var(--text-muted);
  transition: all 0.2s;
}

.clear-btn:hover {
  background: rgba(248,81,73,0.1);
  border-color: #f85149;
  color: #f85149;
}
</style>