#include <SPI.h>
#include <LoRa.h>

// Pinos do LilyGO T-Beam v1.1 para LoRa
#define SCK   5    // SPI Clock
#define MISO 19    // SPI MISO
#define MOSI 27    // SPI MOSI
#define SS   18    // LoRa NSS (CS)
#define RST  14    // LoRa Reset
#define DIO0 26    // LoRa IRQ (DIO0)

// Tamanho máximo do buffer para o pacote recebido
#define BUFFER_SIZE 128

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Iniciando LoRa Receptor...");

  // Inicia o barramento SPI
  SPI.begin(SCK, MISO, MOSI, SS);

  // Configura os pinos do LoRa
  LoRa.setPins(SS, RST, DIO0);

  delay(100);

  // Inicializa o rádio LoRa
  if (!LoRa.begin(433E6)) {
    Serial.println("Erro ao iniciar LoRa!");
    while (1);
  }

  // Configurações LoRa (devem corresponder ao transmissor)
  LoRa.setSpreadingFactor(7);      // SF7
  LoRa.setSignalBandwidth(125E3);  // 125 kHz
  LoRa.setCodingRate4(5);          // 4/5
  LoRa.setTxPower(14);             // 14 dBm
  LoRa.enableCrc();                // Ativa verificação CRC

  Serial.println("Receptor LoRa pronto!");

  // Coloca em modo de recepção contínua
  LoRa.receive();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Buffer para armazenar o pacote recebido
    char buffer[BUFFER_SIZE];
    int index = 0;

    // Lê o pacote LoRa
    while (LoRa.available() && index < BUFFER_SIZE - 1) {
      buffer[index++] = (char)LoRa.read();
    }
    buffer[index] = '\0'; // Termina a string

    // Verifica se o buffer contém dados válidos (não vazio)
    if (index > 0) {
      Serial.println(buffer); // Imprime o conteúdo do pacote
    } else {
      Serial.println("Pacote LoRa vazio ou inválido");
    }
  }
}