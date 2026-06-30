const { Server } = require('socket.io');

console.log('SOCKET.JS CHARGE');

function initSocket(server) {
  const io = new Server(server, {
    cors: {
      origin: '*',
      methods: ['GET', 'POST']
    }
  });

  global.io = io;

  io.on('connection', (socket) => {
    console.log('Frontend connecté au WebSocket');

    socket.on('command', (data) => {
      console.log('Commande reçue du frontend :', data);
    });

    socket.on('disconnect', () => {
      console.log('Frontend déconnecté');
    });
  });

  console.log('RETURN IO');
  return io;
}

module.exports = initSocket;