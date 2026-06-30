# Projet IoT — ESP8266 Dashboard

Systeme de surveillance IoT en temps reel : capteur de temperature/pression (BMP280) sur ESP8266, transmis via MQTT vers un dashboard web (Vue.js + Node.js).

## Architecture

Frontend (Vue.js + PWA)
        |  WebSocket (Socket.IO)
        v
Backend (Node.js + Express)
        |  MQTT
        v
Broker MQTT (broker.hivemq.com)
        |  MQTT
        v
ESP8266 + BMP280 + Ecran OLED

## Structure du projet

Ce projet est organise en 2 parties independantes :

- /backend et /frontend — Application web (dashboard)
  -> voir frontend/README.md pour l'installation

- Code Arduino — Microcontroleur ESP8266
  -> voir le dossier arduino_export, avec son propre README

## Fonctionnalites principales

- Affichage temps reel de la temperature et de la pression
- Simulation de l'ecran OLED (avec debordement de texte visible)
- Graphique interactif (Plotly) avec historique
- Mode offline (PWA) — donnees conservees si connexion perdue
- Configuration dynamique des commandes affichees
- Aide generee automatiquement (toujours synchronisee avec les commandes)
- Adresse du backend configurable
- Detection automatique de nouveaux flux MQTT (sans modifier le code)

## Principe "Netflix" (multi-clients)

Tous les clients connectes recoivent les memes donnees au meme moment, via diffusion MQTT + Socket.IO (io.emit). Teste avec succes sur 2 navigateurs simultanes.

## Topics MQTT

ESP8266_Liza/datalog/temperature
ESP8266_Liza/datalog/pression
ESP8266_Liza/screen/command

Le backend s'abonne avec un wildcard (ESP8266_Liza/datalog/+) pour detecter automatiquement tout nouveau flux.

## Auteurs

AOUAS Liza — Master IoT, 2025/2026