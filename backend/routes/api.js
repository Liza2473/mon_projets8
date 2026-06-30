const express = require('express');
const router = express.Router();

router.get('/status', (req, res) => {
  res.json({
    status: 'OK',
    backend: 'IoT',
    time: new Date()
  });
});

router.post('/command', (req, res) => {
  const { command } = req.body;

  if (!command) {
    return res.status(400).json({ error: 'Commande manquante' });
  }

  res.json({
    success: true,
    command
  });
});

module.exports = router;
