# Application Web — Frontend + Backend

## Prerequis

- Node.js installe (https://nodejs.org)
- Un ESP8266 configure et connecte au meme reseau WiFi (voir code Arduino)

## Installation

### 1. Backend

cd backend
npm install

Configurer le fichier .env si besoin (port, broker MQTT, port serie) :

PORT=3000
MQTT_BROKER=mqtt://broker.hivemq.com
SERIAL_PORT=COM3
SERIAL_BAUD=9600

Lancer le backend :

node server.js

### 2. Frontend

Dans un autre terminal :

cd frontend
npm install
npm run dev

Ouvrir l'adresse affichee (ex: http://localhost:5173)

## Configuration depuis l'interface

- Adresse backend : modifiable directement dans le dashboard (en haut), utile si le backend tourne sur un autre PC/serveur
- Commandes affichees : bouton "Configurer les commandes" -> choisir parmi le catalogue ou ajouter des commandes personnalisees
- Aide : bouton "Aide" -> documentation generee automatiquement

## Mode hors-ligne (PWA)

L'application peut etre installee comme une app (PWA). En cas de perte de connexion, les dernieres donnees recues restent affichees et un bandeau rouge l'indique clairement.