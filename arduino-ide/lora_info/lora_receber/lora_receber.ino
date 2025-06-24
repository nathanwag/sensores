#include <SPI.h>
#include <LoRa.h>

#define SCK   5    // SPI Clock
#define MISO 19    // SPI MISO
#define MOSI 27    // SPI MOSI
#define SS   18    // LoRa NSS (CS)
#define RST  14    // LoRa Reset
#define DIO0 26    // LoRa IRQ (DIO0)

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Inicia o barramento SPI nos pinos do T-Beam
  SPI.begin(SCK, MISO, MOSI, SS);

  // Configura pinos do LoRa (CS, RST, DIO0)
  LoRa.setPins(SS, RST, DIO0);

  // Pequeno delay para garantir startup do módulo
  delay(100);

  // Inicializa rádio na frequência 433 MHz
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed!");
    while (1);
  }
  Serial.println("LoRa init success!");

  // Parâmetros para sincronismo
  LoRa.setSpreadingFactor(7);      // SF7 (7–12) – alcance × sensibilidade
  LoRa.setSignalBandwidth(125E3);  // 125 kHz
  LoRa.setCodingRate4(5);          // 4/5
  LoRa.setTxPower(14);             // 2–17 dBm
  LoRa.enableCrc();                // ativa verificação CRC

  // Coloca em modo de recepção contínua
  LoRa.receive();
}

void loop() {
  // Verifica se chegou pacote
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Pacote Recebido: ");
    // Lê e imprime todo o payload
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }
    // Imprime RSSI para avaliar qualidade de sinal
    Serial.print("  |  RSSI=");
    Serial.println(LoRa.packetRssi());
  }
}
