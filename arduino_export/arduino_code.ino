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

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_BMP280 bmp;

float temp = 0;
float pres = 0;

String screenMode = "ALL";
String modes[]    = { "ALL", "TEMP", "PRESS", "LIST" };
int modeIndex     = 0;
int totalModes    = 4;

bool buttonWasPressed         = false;
unsigned long buttonPressTime = 0;

unsigned long lastSend           = 0;
const unsigned long sendInterval = 5000;

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
// ✅ Publie la metadata (liste des flux) en un seul message JSON
void publishDescriptions() {
  String topicDescribe = deviceID + "/metadata/flows";

  String payload = "{";
  payload += "\"id\":\"" + deviceID + "\",";
  payload += "\"flows\":[";

  // Flux temperature
  payload += "{";
  payload += "\"id\":\"temperature\",";
  payload += "\"label\":\"Temperature\",";
  payload += "\"desc\":\"Mesure la temperature ambiante via le capteur BMP280\",";
  payload += "\"unit\":\"C\"";
  payload += "},";

  // Flux pression
  payload += "{";
  payload += "\"id\":\"pression\",";
  payload += "\"label\":\"Pression atmospherique\",";
  payload += "\"desc\":\"Mesure la pression atmospherique via le capteur BMP280\",";
  payload += "\"unit\":\"hPa\"";
  payload += "}";

  payload += "]}";

  client.publish(topicDescribe.c_str(), payload.c_str());

  Serial.println("Metadata publiee sur : " + topicDescribe);
  Serial.println(payload);
  showText("Metadata OK", "flows envoyes");
  delay(1000);
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
    if (lastCustomMessage != "") {
      display.println(tronquer(lastCustomMessage));
    }
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
    if (lastCustomMessage != "") {
      display.println(tronquer(lastCustomMessage));
    }
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
    display.println("=== ALL FLUX ===");
    display.println(fluxTemp()  ? tronquer("T: " + String(temp, 2) + " C")   : "T: N/A");
    display.println(fluxPress() ? tronquer("P: " + String(pres, 1) + " hPa") : "P: N/A");
    display.println("----------------");
    display.println("MQTT: OK");
    if (lastCustomMessage != "") {
      display.println(tronquer(lastCustomMessage));
    }
  }

  display.display();
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

  // ✅ Commande DESCRIBE → republier la metadata
  if (command == "DESCRIBE") {
    Serial.println("DESCRIBE recu → republication metadata flows");
    publishDescriptions();
    updateOLED();
    return;
  }

  if (command.startsWith("MSG:")) {
    String msg = command.substring(4);
    msg.trim();
    lastCustomMessage = msg;
    Serial.println("Message affiche: " + tronquer(msg));
    updateOLED();
    return;
  }

  if (command.endsWith("?")) {
    String nomFlux = command;
    nomFlux.replace("?", "");
    nomFlux.trim();

    if (nomFlux == "TEMP" || nomFlux == "TEMPERATURE") {
      showText(
        "Flux temperature",
        fluxTemp() ? ">>> OUI <<<" : ">>> NON <<<",
        fluxTemp() ? "Disponible OK" : "Capteur hors plage"
      );
    }
    else if (nomFlux == "PRESS" || nomFlux == "PRESSION") {
      showText(
        "Flux pression",
        fluxPress() ? ">>> OUI <<<" : ">>> NON <<<",
        fluxPress() ? "Disponible OK" : "Capteur hors plage"
      );
    }
    else {
      showText("Flux inconnu", tronquer(nomFlux), ">>> NON <<<");
    }

    delay(3000);
    updateOLED();
    return;
  }

  if (command == "STATUS") {
    showText(
      "=== STATUS ===",
      fluxTemp()  ? "T: OK " + String(temp, 1) + "C"    : "T: N/A",
      fluxPress() ? "P: OK " + String(pres, 0) + " hPa" : "P: N/A"
    );
    delay(3000);
    updateOLED();
    return;
  }

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
    showText("Flux introuvable", tronquer(command), ">>> NON <<<");
    delay(3000);
    updateOLED();
    return;
  }

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
    updateOLED();
  }
}

// -----------------------------------------------
void publishData() {
  String topicTemp = deviceID + "/datalog/temperature";
  String topicPres = deviceID + "/datalog/pression";

  String messageTemp = "{";
  messageTemp += "\"id\":\"" + deviceID + "\",";
  messageTemp += "\"messageId\":\"temp-" + String(millis()) + "\",";
  messageTemp += "\"datalog\":{";
  messageTemp += "\"type\":\"temperature\",";
  messageTemp += "\"value\":" + String(temp, 2) + ",";
  messageTemp += "\"unit\":\"C\",";
  messageTemp += "\"timestamp\":" + String(millis());
  messageTemp += "}}";

  String messagePres = "{";
  messagePres += "\"id\":\"" + deviceID + "\",";
  messagePres += "\"messageId\":\"press-" + String(millis()) + "\",";
  messagePres += "\"datalog\":{";
  messagePres += "\"type\":\"pression\",";
  messagePres += "\"value\":" + String(pres, 1) + ",";
  messagePres += "\"unit\":\"hPa\",";
  messagePres += "\"timestamp\":" + String(millis());
  messagePres += "}}";

  client.publish(topicTemp.c_str(), messageTemp.c_str());
  client.publish(topicPres.c_str(), messagePres.c_str());

  Serial.println("Temp: " + String(temp, 2) + " C");
  Serial.println("Pres: " + String(pres, 1) + " hPa");
}

// -----------------------------------------------
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connexion MQTT...");
    showText("MQTT connexion...");

    String clientId = deviceID + "_" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connecte !");

      // Souscription aux commandes
      String topicCmd = deviceID + "/screen/command";
      client.subscribe(topicCmd.c_str());
      Serial.println("Abonne a: " + topicCmd);

      // ✅ Publier la metadata dès la connexion
      publishDescriptions();

      showText("MQTT OK", "Commandes actives", tronquer(topicCmd));
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

  if (!bmp.begin(0x77)) {
    Serial.println("BMP280 non detecte !");
    while (1);
  }

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

  if (millis() - lastSend >= sendInterval) {
    lastSend = millis();
    temp = bmp.readTemperature();
    pres = bmp.readPressure() / 100.0F;
    updateOLED();
    publishData();
  }
}