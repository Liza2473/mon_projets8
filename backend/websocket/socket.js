const { Server } = require('socket.io')
const { sendToSerial } = require('../serial/serial')

function initSocket(server, mqttClient) {
  const io = new Server(server, {
    cors: {
      origin: '*',
      methods: ['GET', 'POST']
    }
  })

  io.on('connection', (socket) => {
    console.log('✅ Frontend connecté au WebSocket')

    socket.on('command', (data) => {
      const action = data?.action?.trim().toUpperCase()

      if (!action) return

      console.log('📩 Commande écran reçue:', action)

      sendToSerial(action)

      if (mqttClient && mqttClient.connected) {
        const topic = 'ESP8266_Liza/screen/command'
        mqttClient.publish(topic, action)
        console.log(`📡 Commande MQTT envoyée: ${topic} → ${action}`)
      } else {
        console.warn('⚠️ MQTT non connecté')
      }
    })

    socket.on('disconnect', () => {
      console.log('❌ Frontend déconnecté')
    })
  })

  return io
}

module.exports = initSocket