<template>
  <div class="header card">
    <h1>IoT Controller</h1>

    <button
      class="connect-btn"
      :class="{ connected: isConnected }"
      @click="$emit('toggle-connection')"
    >
      {{ isConnected ? 'Se déconnecter' : 'Se connecter' }}
    </button>
  </div>
</template>

<script setup>
defineProps({
  isConnected: Boolean
})

defineEmits(['toggle-connection'])
</script>

<style scoped>
.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.connect-btn {
  padding: 10px 18px;
  border-radius: 12px;
  border: none;
  cursor: pointer;
  background: var(--primary);
  color: #000;
  font-weight: 600;
}

.connect-btn.connected {
  background: #22c55e;
}
</style>