<template>
  <footer class="footer">
    <div class="footer-left">
      <span class="footer-dot" :class="backendStatus === 'Connecté' ? 'dot-green' : 'dot-red'"></span>
      <span class="footer-item">{{ backendUrl }}</span>
    </div>
    <div class="footer-center">
      © 2026 — IoT Controller • AOUAS Liza — Master IoT 2025/2026
    </div>
    <div class="footer-right">
      <span class="footer-item">v1.0.0</span>
    </div>
  </footer>
</template>

<script setup>
defineProps({
  backendUrl:    { type: String, default: 'http://localhost:3000' },
  backendStatus: { type: String, default: 'Déconnecté' }
})
</script>

<style scoped>
.footer {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 10px 16px;
  background: var(--card);
  border: 1px solid var(--border);
  border-radius: 10px;
  margin-top: 4px;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
}

.footer-left, .footer-right {
  display: flex;
  align-items: center;
  gap: 6px;
  flex: 1;
}

.footer-right { justify-content: flex-end; }

.footer-center {
  font-size: 12px;
  color: var(--text-hint);
  text-align: center;
  flex: 2;
}

.footer-item {
  font-size: 11px;
  color: var(--text-hint);
  font-family: monospace;
}

.footer-dot {
  width: 7px;
  height: 7px;
  border-radius: 50%;
  flex-shrink: 0;
}

.dot-green {
  background: var(--green);
  box-shadow: 0 0 6px var(--green);
}

.dot-red {
  background: var(--red);
  box-shadow: 0 0 6px var(--red);
}
</style>