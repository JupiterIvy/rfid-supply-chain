#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>

// Configurações da rede Wi-Fi
const char* ssid = "Dj cleiton rasta";
const char* password = "Mei300301";

// Configurações do broker MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* topic = "supplychain/rfid";

// Nome da localização deste leitor
const char* reader_location = "Armazem";

WiFiClient espClient;
PubSubClient client(espClient);

// Pinos do leitor RFID
#define RST_PIN 22
#define SS_PIN 21

MFRC522 rfid(SS_PIN, RST_PIN);
unsigned long lastReadTime = 0;
String lastCardUID = "";

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado.");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado.");
    } else {
      Serial.print("Erro, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("Leitor RFID pronto.");
}

String getCardUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

void sendMQTTMessage(const String& uid) {
  StaticJsonDocument<200> doc;
  doc["product_id"] = uid;
  doc["location"] = reader_location;

  JsonObject details = doc.createNestedObject("details");
  details["name"] = uid;
  details["category"] = "fruit";

  char buffer[256];
  size_t n = serializeJson(doc, buffer);
  client.publish(topic, buffer, n);

  Serial.println("📤 Dados enviados via MQTT:");
  Serial.println(buffer);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String uid = getCardUID();

  // Evita leituras duplicadas em menos de 5 segundos
  if (uid == lastCardUID && (millis() - lastReadTime < 5000)) {
    return;
  }

  lastCardUID = uid;
  lastReadTime = millis();

  sendMQTTMessage(uid);

  rfid.PICC_HaltA();
}