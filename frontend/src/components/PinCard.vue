<template>
  <button class="pin-card" :class="{ active }" @click="emit('toggle')">
    <div class="icon">{{ label }}</div>
    <p>{{ title }}</p>
  </button>
</template>

<script setup>
defineProps({
  label: String,
  title: String,
  active: Boolean
})

const emit = defineEmits(['toggle'])
</script>

<style scoped>
.pin-card {
  width: 160px;
  text-align: center;
  padding: 22px;
  border-radius: 18px;
  background: rgba(255,255,255,0.03);
  border: 1px solid var(--border-soft);
  cursor: pointer;
  transition: 0.3s;
  color: inherit;
}

.pin-card:hover {
  transform: translateY(-4px);
}

.pin-card.active {
  border-color: var(--primary);
  box-shadow: 0 0 25px rgba(56,189,248,0.25);
}

.icon {
  width: 60px;
  height: 60px;
  margin: 0 auto 10px;
  border-radius: 14px;
  background: #020617;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--primary);
  font-weight: bold;
}

p {
  color: var(--text-muted);
}
</style>