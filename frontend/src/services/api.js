let connected = false

export function connect() {
  connected = true
  return 'Connecté au serveur'
}

export function disconnect() {
  connected = false
  return 'Déconnecté du serveur'
}

export function isConnected() {
  return connected
}

export function sendCommand(command) {
  if (!connected) {
    return 'Erreur : non connecté'
  }
  return `Commande exécutée : ${command}`
}

export function getSensorData() {
  if (!connected) {
    return {
      temperature: '--',
      humidity: '--',
      voltage: '--'
    }
  }

  return {
    temperature: (20 + Math.random() * 5).toFixed(1),
    humidity: (40 + Math.random() * 10).toFixed(1),
    voltage: (3.3 + Math.random() * 0.1).toFixed(2),
  }
}
