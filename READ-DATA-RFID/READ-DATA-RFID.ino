#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>

// Configurações da rede Wi-Fi
const char* ssid = "SSID";
const char* password = "PASSWORD";

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
String lastCardContent = "";

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

String readCardContent() {
  byte block = 4; // Bloco onde os dados foram gravados
  byte buffer[18]; // Buffer para receber dados
  byte length = 18;

  MFRC522::StatusCode status = rfid.MIFARE_Read(block, buffer, &length);

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Erro na leitura do bloco: ");
    Serial.println(rfid.GetStatusCodeName(status));
    return "";
  }

  String content = "";
  for (int i = 0; i < 16; i++) { // Um bloco tem 16 bytes
    if (buffer[i] != 0) {
      content += (char)buffer[i];
    }
  }
  content.trim(); // Remove espaços vazios no final, se houver
  return content;
}

void sendMQTTMessage(const String& product_id, const String& name) {
  StaticJsonDocument<300> doc;
  
  doc["product_id"] = product_id;
  
  JsonObject details = doc.createNestedObject("details");
  details["name"] = name;
  details["category"] = "fruit"; // categoria fixa

  doc["location"] = reader_location;

  char buffer[512];
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

  String cardContent = readCardContent();

  if (cardContent == "") {
    Serial.println("⚠️ Conteúdo vazio ou erro ao ler a tag.");
    rfid.PICC_HaltA();
    return;
  }

  // Evita leituras duplicadas em menos de 5 segundos
  if (cardContent == lastCardContent && (millis() - lastReadTime < 5000)) {
    rfid.PICC_HaltA();
    return;
  }

  lastCardContent = cardContent;
  lastReadTime = millis();

  int separatorIndex = cardContent.indexOf('\n');
  if (separatorIndex == -1) {
    Serial.println("⚠️ Conteúdo inválido! Esperava uma quebra de linha separando product_id e nome.");
    rfid.PICC_HaltA();
    return;
  }

  String product_id = cardContent.substring(0, separatorIndex);
  String name = cardContent.substring(separatorIndex + 1);

  sendMQTTMessage(product_id, name);

  rfid.PICC_HaltA();
}
