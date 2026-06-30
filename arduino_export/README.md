# Code Microcontroleur — ESP8266

## Materiel necessaire

- ESP8266 (NodeMCU ou Wemos D1)
- Capteur BMP280 (temperature + pression)
- Ecran OLED SSD1306 128x64
- Bouton poussoir (mode d'affichage)

## Branchements (I2C)

- SDA -> GPIO 14 (D5)
- SCL -> GPIO 12 (D6)
- Bouton -> GPIO 0 (D3), avec pull-up interne

## Bibliotheques Arduino requises

Installer via le gestionnaire de bibliotheques Arduino IDE :
- PubSubClient (MQTT)
- Adafruit GFX Library
- Adafruit SSD1306
- Adafruit BMP280 Library

## Configuration

Avant de flasher, modifier dans le code :

const char* ssid     = "NOM_DU_WIFI";
const char* password = "MOT_DE_PASSE_WIFI";
const String deviceID = "ESP8266_Liza";

## Flash

1. Ouvrir arduino_code.ino dans Arduino IDE
2. Selectionner la carte ESP8266 correspondante
3. Selectionner le port COM
4. Cliquer sur "Televerser" (Upload)

## Topics MQTT publies

ESP8266_Liza/datalog/temperature
ESP8266_Liza/datalog/pression

Toutes les 5 secondes (sans bloquer la boucle principale, via millis()).

## Commandes recues (topic ESP8266_Liza/screen/command)

- LIST, ALL, STATUS, TEMP, PRESS
- TEMP?, PRESS?, ou toute commande se terminant par ? -> repond OUI/NON selon disponibilite reelle du capteur
- MSG:texte -> affiche un message personnalise sur l'ecran
- MSG:CLEAR -> efface le message personnalise

Toute commande inconnue recoit la reponse "Flux introuvable / NON" (l'appareil ne ment jamais sur ses capacites reelles).