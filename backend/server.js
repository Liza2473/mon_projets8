require('dotenv').config();

const mqtt    = require('mqtt');
const express = require('express');
const cors    = require('cors');
const http    = require('http');

const { initSerial } = require('./serial/serial');
const initSocket     = require('./websocket/socket');
const apiRoutes      = require('./routes/api');

const app = express();

const PORT        = process.env.PORT        || 3000;
const MQTT_BROKER = process.env.MQTT_BROKER || 'mqtt://broker.hivemq.com';
const SERIAL_PORT = process.env.SERIAL_PORT || 'COM3';
const SERIAL_BAUD = parseInt(process.env.SERIAL_BAUD) || 9600;

app.use(cors());
app.use(express.json());
app.use('/api', apiRoutes);

app.get('/', (req, res) => {
  res.json({ message: 'Backend IoT en marche' });
});

const server = http.createServer(app);

// ✅ Format 4 blocs : deviceId/datalog/{timestamp}/{fluxName}
const TOPIC_DATALOG  = '+/datalog/+/+';
const TOPIC_METADATA = '+/metadata/flows';

// ✅ ClientId unique pour éviter les refus de connexion
const mqttClient = mqtt.connect(MQTT_BROKER, {
  clientId: 'backend_' + Math.random().toString(16).substring(2, 8),
  clean: true,
  reconnectPeriod: 5000
});

const io = initSocket(server, mqttClient);

mqttClient.on('connect', () => {
  console.log('✅ Connecte au broker MQTT');

  mqttClient.subscribe(TOPIC_DATALOG, (err) => {
    if (err) {
      console.error('❌ Erreur abonnement datalog:', err.message);
    } else {
      console.log('📡 Abonne au topic datalog  :', TOPIC_DATALOG);
    }
  });

  mqttClient.subscribe(TOPIC_METADATA, (err) => {
    if (err) {
      console.error('❌ Erreur abonnement metadata:', err.message);
    } else {
      console.log('📡 Abonne au topic metadata :', TOPIC_METADATA);
    }
  });

  // Demander la metadata à l'Arduino après connexion
  setTimeout(() => {
    mqttClient.publish('ESP8266_Liza/screen/command', 'DESCRIBE');
    console.log('📤 Commande DESCRIBE envoyée → Arduino va republier ses flux');
  }, 5000);
});

mqttClient.on('message', (topic, message) => {
  try {
    const data  = JSON.parse(message.toString());
    const parts = topic.split('/');

    // ✅ Metadata : deviceId/metadata/flows
    if (parts[1] === 'metadata' && parts[2] === 'flows') {
      const deviceId = parts[0];
      console.log('📋 Metadata reçue depuis :', deviceId);
      console.log('   Flux disponibles :', data.flows.map(f => f.id).join(', '));
      io.emit('flux-metadata', data);
      console.log('➡️ Metadata envoyée au frontend : flux-metadata');
      return;
    }

    // ✅ Données capteurs : deviceId/datalog/{timestamp}/{fluxName}
    if (parts[1] === 'datalog' && parts.length === 4) {
      const deviceId  = parts[0];
      const timestamp = parts[2];
      const fluxName  = parts[3];

      if (!fluxName) {
        console.warn('⚠️ Flux invalide:', topic);
        return;
      }

      console.log('📨 Flux :', fluxName, '| Appareil :', deviceId, '| ts :', timestamp);

      io.emit(fluxName + '-data', {
        deviceId,
        timestamp,
        flux: fluxName,
        value: data.value,
        unit: data.unit
      });

      console.log('➡️ Envoye au frontend :', fluxName + '-data');
      return;
    }

    console.warn('⚠️ Topic non reconnu:', topic);

  } catch (error) {
    console.error('❌ Erreur parsing JSON MQTT:', error.message);
  }
});

mqttClient.on('error', (err) => {
  console.error('❌ Erreur MQTT:', err.message);
});

mqttClient.on('offline', () => {
  console.warn('⚠️ MQTT hors ligne, tentative de reconnexion...');
});

initSerial(SERIAL_PORT, SERIAL_BAUD);

server.listen(PORT, () => {
  console.log(`🚀 Backend demarre sur http://localhost:${PORT}`);
});