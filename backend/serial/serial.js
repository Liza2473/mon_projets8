const { SerialPort } = require('serialport');
const EventEmitter = require('events');

const serialEvents = new EventEmitter();
let port = null;

function initSerial(portPath = 'COM3', baudRate = 115200) {
  port = new SerialPort({ path: portPath, baudRate });

  port.on('open', () => {
    console.log('✅ ESP32 connecté sur', portPath);
  });

  port.on('data', (data) => {
    const message = data.toString().trim();
    console.log('📟 ESP →', message);
    serialEvents.emit('data', message);
  });

  port.on('error', (err) => {
    console.log('❌ Erreur série:', err.message);
  });
}

function sendToSerial(message) {
  if (port && port.isOpen) {
    port.write(message + '\n');
  }
}

module.exports = {
  initSerial,
  sendToSerial,
  serialEvents
};
