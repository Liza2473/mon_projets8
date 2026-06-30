#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define MAX_CHARS 21
#define BUTTON_PIN 0
#define LONG_PRESS 1500

const char* ssid        = "iPhone";
const char* password    = "lizaLiza";
const char* mqtt_server = "broker.hivemq.com";

const String deviceID = "ESP8266_Liza";

WiFiClient   espClient;
PubSubClient client(espClient);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_BMP280  bmp;

float  temp = 0;
float  pres = 0;

String screenMode = "ALL";
String modes[]    = { "ALL", "TEMP", "PRESS", "LIST" };
int    modeIndex  = 0;
int    totalModes = 4;

bool          buttonWasPressed = false;
unsigned long buttonPressTime  = 0;

unsigned long lastSend     = 0;
unsigned long lastMetaSend = 0;
unsigned long lastOledRefresh = 0;  // ✅ Nouveau timer pour rafraîchir OLED

const unsigned long sendInterval  = 5000;
const unsigned long metaInterval  = 30000;
const unsigned long oledInterval  = 5000;  // ✅ Rafraîchit OLED toutes les 5s

// ✅ Flag pour bloquer le rafraîchissement auto pendant un message temporaire
bool oledLocked = false;
unsigned long oledLockUntil = 0;

String lastCustomMessage = "";

// -----------------------------------------------
String tronquer(String texte) {
  if ((int)texte.length() > MAX_CHARS)
    return texte.substring(0, MAX_CHARS - 2) + "..";
  return texte;
}

bool fluxTemp()  { return temp > -40 && temp < 85; }
bool fluxPress() { return pres > 300 && pres < 1200; }

// -----------------------------------------------
void showText(String line1, String line2 = "", String line3 = "") {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(tronquer(line1));
  if (line2 != "") { display.setCursor(0, 18); display.println(tronquer(line2)); }
  if (line3 != "") { display.setCursor(0, 36); display.println(tronquer(line3)); }
  display.display();
}

// -----------------------------------------------
void updateOLED() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);

  if (screenMode == "TEMP") {
    display.println("Flux: Temperature");
    display.println("----------------");
    if (fluxTemp()) {
      display.println(tronquer("Val: " + String(temp, 2) + " C"));
      display.println("Status: OK");
    } else {
      display.println("Status: N/A");
      display.println("Hors plage");
    }
    if (lastCustomMessage != "") display.println(tronquer(lastCustomMessage));
  }

  else if (screenMode == "PRESS") {
    display.println("Flux: Pression");
    display.println("----------------");
    if (fluxPress()) {
      display.println(tronquer("Val: " + String(pres, 1) + " hPa"));
      display.println("Status: OK");
    } else {
      display.println("Status: N/A");
      display.println("Hors plage");
    }
    if (lastCustomMessage != "") display.println(tronquer(lastCustomMessage));
  }

  else if (screenMode == "LIST") {
    display.println("Flux disponibles");
    display.println("----------------");
    display.println(fluxTemp()  ? "+ temperature OK" : "- temperature N/A");
    display.println(fluxPress() ? "+ pression OK"    : "- pression N/A");
    display.println("----------------");
    if (lastCustomMessage != "") {
      display.println(tronquer(lastCustomMessage));
    } else {
      display.println("Total: 2 flux");
    }
  }

  else {
    // ALL
    display.println("=== ALL FLUX ===");
    display.println(fluxTemp()  ? tronquer("T: " + String(temp, 2) + " C")   : "T: N/A");
    display.println(fluxPress() ? tronquer("P: " + String(pres, 1) + " hPa") : "P: N/A");
    display.println("----------------");
    display.println("MQTT: OK");
    if (lastCustomMessage != "") display.println(tronquer(lastCustomMessage));
  }

  display.display();
}

// -----------------------------------------------
void publishDescriptions() {
  String topicDescribe = deviceID + "/metadata/flows";

  String payload = "{";
  payload += "\"id\":\"" + deviceID + "\",";
  payload += "\"flows\":[";
  payload += "{\"id\":\"temperature\",\"label\":\"Temperature\",\"desc\":\"Capteur BMP280\",\"unit\":\"C\"},";
  payload += "{\"id\":\"pression\",\"label\":\"Pression\",\"desc\":\"Capteur BMP280\",\"unit\":\"hPa\"}";
  payload += "]}";

  bool ok = client.publish(topicDescribe.c_str(), payload.c_str(), true);

  if (ok) {
    Serial.println("Metadata OK");
  } else {
    Serial.println("ERREUR metadata");
  }
}

// -----------------------------------------------
void publishData() {
  String ts = String(millis());

  String topicTemp = deviceID + "/datalog/" + ts + "/temperature";
  String topicPres = deviceID + "/datalog/" + ts + "/pression";

  String msgTemp = "{\"value\":" + String(temp, 2) + ",\"unit\":\"C\"}";
  String msgPres = "{\"value\":" + String(pres, 1) + ",\"unit\":\"hPa\"}";

  client.publish(topicTemp.c_str(), msgTemp.c_str());
  client.publish(topicPres.c_str(), msgPres.c_str());

  Serial.println("Temp: " + String(temp, 2) + " C");
  Serial.println("Pres: " + String(pres, 1) + " hPa");
}

// -----------------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  String command = "";
  for (unsigned int i = 0; i < length; i++) {
    command += (char)payload[i];
  }
  command.trim();
  command.toUpperCase();

  Serial.println("Commande recue: " + command);

  // DESCRIBE — republier les métadonnées
  if (command == "DESCRIBE") {
    publishDescriptions();
    // ✅ Ne change pas screenMode, juste confirme brièvement
    showText("DESCRIBE OK", "Metadata envoye");
    oledLocked    = true;
    oledLockUntil = millis() + 2000;
    return;
  }

  // MSG: — message personnalisé
  if (command.startsWith("MSG:")) {
    String msg = command.substring(4);
    msg.trim();
    lastCustomMessage = msg;
    updateOLED();
    return;
  }

  // ✅ TEMP? PRESS? — affiche 3s PUIS revient au screenMode actuel
  if (command.endsWith("?")) {
    String nomFlux = command;
    nomFlux.replace("?", "");
    nomFlux.trim();

    if (nomFlux == "TEMP" || nomFlux == "TEMPERATURE") {
      showText("Flux temperature",
        fluxTemp() ? ">>> OUI <<<" : ">>> NON <<<",
        fluxTemp() ? "Disponible OK" : "Hors plage");
    }
    else if (nomFlux == "PRESS" || nomFlux == "PRESSION") {
      showText("Flux pression",
        fluxPress() ? ">>> OUI <<<" : ">>> NON <<<",
        fluxPress() ? "Disponible OK" : "Hors plage");
    }
    else {
      showText("Flux inconnu", tronquer(nomFlux), ">>> NON <<<");
    }
    //  Lock l'OLED 3s puis updateOLED remet le bon screenMode
    oledLocked    = true;
    oledLockUntil = millis() + 3000;
    return;
  }

  //  STATUS — affiche 3s puis revient au screenMode actuel
  if (command == "STATUS") {
    showText("=== STATUS ===",
      fluxTemp()  ? "T: OK " + String(temp, 1) + "C"    : "T: N/A",
      fluxPress() ? "P: OK " + String(pres, 0) + " hPa" : "P: N/A");
    oledLocked    = true;
    oledLockUntil = millis() + 3000;
    return;
  }

  // ✅ Commandes de mode — changent screenMode et RESTENT
  if (command == "ALL") {
    screenMode = "ALL";
    modeIndex  = 0;
  }
  else if (command == "TEMP" || command == "TEMPERATURE") {
    screenMode = "TEMP";
    modeIndex  = 1;
  }
  else if (command == "PRESS" || command == "PRESSION") {
    screenMode = "PRESS";
    modeIndex  = 2;
  }
  else if (command == "LIST") {
    screenMode = "LIST";
    modeIndex  = 3;
  }
  else {
    // Commande inconnue — affiche 3s puis revient
    showText("Flux introuvable", tronquer(command), ">>> NON <<<");
    oledLocked    = true;
    oledLockUntil = millis() + 3000;
    return;
  }

  // Mise à jour immédiate de l'écran avec le nouveau mode
  oledLocked = false;
  updateOLED();
}

// -----------------------------------------------
void handleButton() {
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  if (buttonPressed && !buttonWasPressed) {
    buttonPressTime  = millis();
    buttonWasPressed = true;
  }

  if (!buttonPressed && buttonWasPressed) {
    unsigned long duration = millis() - buttonPressTime;
    buttonWasPressed = false;

    if (duration >= LONG_PRESS) {
      modeIndex  = 0;
      screenMode = "ALL";
      showText("Mode: ALL", "Reset OK");
    } else {
      modeIndex  = (modeIndex + 1) % totalModes;
      screenMode = modes[modeIndex];
      showText("Mode:", screenMode);
    }

    delay(200);
    oledLocked = false;
    updateOLED();
  }
}

// -----------------------------------------------
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connexion MQTT...");
    showText("MQTT connexion...");

    String clientId = deviceID + "_" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connecte !");

      String topicCmd = deviceID + "/screen/command";
      client.subscribe(topicCmd.c_str());
      Serial.println("Abonne a: " + topicCmd);

      publishDescriptions();
      lastMetaSend = millis();

      showText("MQTT OK", "Commandes actives");
      delay(1000);

    } else {
      Serial.print("echec rc=");
      Serial.println(client.state());
      showText("MQTT erreur", "rc=" + String(client.state()), "Retry 2s...");
      delay(2000);
    }
  }
}

// -----------------------------------------------
void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin(14, 12);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Ecran non detecte !");
    while (1);
  }

  if (!bmp.begin(0x76) && !bmp.begin(0x77)) {
    Serial.println("BMP280 non detecte !");
    showText("BMP280 ERR", "Verif branchement");
    while (1);
  }
  Serial.println("BMP280 detecte OK !");

  showText("=== IoT Liza ===", "Connexion WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connecte !");
  Serial.println(WiFi.localIP());

  showText("WiFi OK", WiFi.localIP().toString(), "MQTT...");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// -----------------------------------------------
void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();
  handleButton();

  //  Republication métadonnées périodique
  if (millis() - lastMetaSend >= metaInterval) {
    lastMetaSend = millis();
    publishDescriptions();
    Serial.println("Metadata republication periodique");
  }

  //  Lecture capteur + publication MQTT
  if (millis() - lastSend >= sendInterval) {
    lastSend = millis();
    temp = bmp.readTemperature();
    pres = bmp.readPressure() / 100.0F;
    publishData();
    //  NE PAS appeler updateOLED() ici
    //    L'OLED se rafraîchit via son propre timer ci-dessous
  }

  //  Rafraîchissement OLED — seulement si pas locké
  if (millis() - lastOledRefresh >= oledInterval) {
    lastOledRefresh = millis();

    // Si le lock est expiré, on déverrouille
    if (oledLocked && millis() >= oledLockUntil) {
      oledLocked = false;
    }

    // Rafraîchit l'écran seulement si pas de message temporaire
    if (!oledLocked) {
      updateOLED();
    }
  }
}