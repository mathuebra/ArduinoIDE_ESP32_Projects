#include <SPI.h>
#include <MFRC522.h>


// Defina os pinos conforme sua configuração
#define SS_PIN 5  // Pino SDA (SS) no ESP32 ou ESP8266
#define RST_PIN 4 // Pino RST no ESP32 ou ESP8266

MFRC522 rfid(SS_PIN, RST_PIN); // Cria a instância do objeto MFRC522

void setup() {
  Serial.begin(115200); // Inicializa a comunicação serial
  SPI.begin();          // Inicializa a comunicação SPI
  rfid.PCD_Init();      // Inicializa o módulo RFID

  Serial.println("Aproxime o cartão RFID para testar.");
}

void loop() {
  // Verifica se há um novo cartão presente
  if (!rfid.PICC_IsNewCardPresent()) return;

  // Tenta ler o cartão
  if (!rfid.PICC_ReadCardSerial()) return;

  // Exibe o UID do cartão
  Serial.print("UID do cartão: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Para a leitura do cartão
  rfid.PICC_HaltA();
}
