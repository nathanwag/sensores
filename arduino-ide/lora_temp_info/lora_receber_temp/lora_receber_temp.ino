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
    String payload = "";

    while (LoRa.available()) {
      payload += (char)LoRa.read();
    }

    Serial.println("Recebido: " + payload + " (RSSI " + LoRa.packetRssi() + ")");

    // Quebra a string por espaço
    float values[5] = {0}; // temperatura, umidade, pressão, altitude, gás
    int valueIndex = 0;
    int startIndex = 0;

    while (valueIndex < 5) {
      int spaceIndex = payload.indexOf(' ', startIndex);
      String part;

      if (spaceIndex == -1) {
        // Último valor ou único valor
        part = payload.substring(startIndex);
      } else {
        part = payload.substring(startIndex, spaceIndex);
        startIndex = spaceIndex + 1;
      }

      values[valueIndex++] = part.toFloat();

      if (spaceIndex == -1) break; // não há mais espaços
    }

    // Mostra os valores com unidade
    Serial.println("----- Dados Formatados -----");
    Serial.printf("Temperatura: %.1f °C\n", values[0]);
    Serial.printf("Umidade: %.1f %%\n", values[1]);
    Serial.printf("Pressão: %.0f hPa\n", values[2]);
    Serial.printf("Altitude: %.1f m\n", values[3]);
    Serial.printf("Gás: %.1f ppm\n", values[4]);
  }
}
