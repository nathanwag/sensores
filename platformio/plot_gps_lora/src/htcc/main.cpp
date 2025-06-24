#include <Arduino.h>
#include "GPS_Air530Z.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include <math.h>
#include "LoRaWan_APP.h" // Biblioteca para o LoRa

// Configurações do display OLED
SSD1306Wire display(0x3c, 500000, SDA, SCL, GEOMETRY_128_64, GPIO10);
Air530ZClass GPS;

// Configurações do LoRa
#define RF_FREQUENCY                433000000 // Frequência em Hz (exemplo: 915 MHz)
#define TX_OUTPUT_POWER             14        // Potência de transmissão em dBm
#define LORA_BANDWIDTH              0         // 0: 125 kHz
#define LORA_SPREADING_FACTOR       7         // SF7
#define LORA_CODINGRATE             1         // 4/5
#define LORA_PREAMBLE_LENGTH        8         // Preâmbulo
#define LORA_FIX_LENGTH_PAYLOAD_ON  false     // Tamanho fixo do payload
#define LORA_IQ_INVERSION_ON        false     // Inversão de IQ
#define BUFFER_SIZE                 128       // Tamanho máximo do pacote LoRa

char txpacket[BUFFER_SIZE];              // Buffer para o pacote LoRa
static RadioEvents_t RadioEvents;        // Eventos de rádio LoRa
bool lora_idle = true;                   // Flag para indicar se o LoRa está pronto

// Função para ligar a alimentação externa (Vext)
void VextON(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

// Função para desligar a alimentação externa (Vext)
void VextOFF(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
}

// Callback para quando a transmissão LoRa é concluída
void OnTxDone(void) {
  Serial.println("Transmissão LoRa concluída.");
  lora_idle = true;
}

// Callback para quando a transmissão LoRa excede o tempo
void OnTxTimeout(void) {
  Serial.println("Tempo de transmissão LoRa excedido.");
  Radio.Sleep();
  lora_idle = true;
}

void setup() {
  VextON();
  delay(10);

  // Inicializar o display OLED
  display.init();
  display.clear();
  display.display();

  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 32 - 16 / 2, "GPS initing...");
  display.display();

  // Inicializar comunicação serial e GPS
  Serial.begin(115200);
  GPS.begin();

  // Inicializar LoRa
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
}

void loop() {
  // Capturar dados do GPS por 1 segundo
  uint32_t starttime = millis();
  while ((millis() - starttime) < 1000) {
    while (GPS.available() > 0) {
      GPS.encode(GPS.read());
    }
  }

  // Variáveis para formatar os dados
  char str[30];
  char latStr[15], lonStr[15], altStr[15], hdopStr[15], speedStr[15];

  // Formatar os valores do GPS como strings
  dtostrf(GPS.location.lat(), 9, 6, latStr);
  dtostrf(GPS.location.lng(), 9, 6, lonStr);
  dtostrf(GPS.altitude.meters(), 7, 2, altStr);
  dtostrf(GPS.hdop.hdop(), 5, 2, hdopStr);
  dtostrf(GPS.speed.kmph(), 7, 3, speedStr);

  // Atualizar o display OLED
  display.clear();
  display.setFont(ArialMT_Plain_10);
  int index = sprintf(str, "%02d-%02d-%02d", GPS.date.year(), GPS.date.day(), GPS.date.month());
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, str);

  index = sprintf(str, "%02d:%02d:%02d", GPS.time.hour(), GPS.time.minute(), GPS.time.second());
  display.drawString(60, 0, str);

  display.drawString(120, 0, GPS.location.age() < 1000 ? "A" : "V");

  display.drawString(0, 16, String("alt: ") + altStr);
  display.drawString(0, 32, String("hdop: ") + hdopStr);
  display.drawString(60, 16, String("lat: ") + latStr);
  display.drawString(60, 32, String("lon: ") + lonStr);
  display.drawString(0, 48, String("speed: ") + speedStr + " km/h");
  display.display();

  // Enviar dados via LoRa se o rádio estiver pronto
  if (lora_idle) {
    // Montar o pacote no formato desejado
    snprintf(txpacket, BUFFER_SIZE, "%s,%s",
             latStr, lonStr);

    // Exibir o pacote no monitor serial para debug
    Serial.printf("\r\nEnviando pacote LoRa: \"%s\", tamanho %d\r\n", txpacket, strlen(txpacket));
    
    // Enviar o pacote via LoRa
    Radio.Send((uint8_t *)txpacket, strlen(txpacket));
    lora_idle = false; // Aguardar a conclusão da transmissão
  }
}