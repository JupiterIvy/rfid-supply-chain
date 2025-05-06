#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>

// === CONFIGURAÇÕES DE WIFI ===
const char* ssid = "SSID";         // << Coloque entre aspas
const char* password = "PW";    // << Coloque entre aspas

// === CONFIGURAÇÕES DE MQTT ===
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* topic = "supplychain/rfid";
const char* reader_location = "Armazem";

// === OBJETOS GLOBAIS ===
WiFiClient espClient;
PubSubClient client(espClient);

// === CONFIGURAÇÃO DO RFID RC522 ===
#define RST_PIN 22
#define SS_PIN 21
MFRC522 rfid(SS_PIN, RST_PIN);

unsigned long lastReadTime = 0;
String lastCardUID = "";

void setup_wifi() {
  Serial.println("\nConectando-se ao WiFi...");
  WiFi.begin(ssid, password);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Falha ao conectar ao WiFi.");
  }
}

void reconnect() {
  // Gera um nome único para o cliente MQTT usando o chip ID
  String clientId = "ESP32Client-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  while (!client.connected()) {
    Serial.print("Conectando ao MQTT... ");
    if (client.connect(clientId.c_str())) {
      Serial.println("✅ Conectado ao broker MQTT.");
    } else {
      Serial.print("❌ Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5s...");
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
  Serial.println("🟢 Leitor RFID pronto.");
}

String getCardUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase(); // Corrigido: precisa reassociar se quiser usar a versão maiúscula
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

  if (client.publish(topic, buffer, n)) {
    Serial.println("📤 Dados enviados via MQTT:");
    Serial.println(buffer);
  } else {
    Serial.println("❌ Falha ao publicar no tópico MQTT.");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❗ WiFi desconectado, tentando reconectar...");
    setup_wifi();
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uid = getCardUID();

  if (uid == lastCardUID && (millis() - lastReadTime < 5000)) {
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }

  lastCardUID = uid;
  lastReadTime = millis();

  Serial.print("🔍 UID detectado: ");
  Serial.println(uid);

  sendMQTTMessage(uid);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
