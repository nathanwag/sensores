#include "LoRaWan_APP.h"
#include "Arduino.h"

#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

#define RF_FREQUENCY               433000000 // Frequência em Hz
#define TX_OUTPUT_POWER            14        // Potência de transmissão em dBm

#define LORA_BANDWIDTH             0         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz]
#define LORA_SPREADING_FACTOR      7         // [SF7..SF12]
#define LORA_CODINGRATE            1         // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH       8         // Mesmo para Tx e Rx
#define LORA_SYMBOL_TIMEOUT        0         // Símbolos
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON       false

#define RX_TIMEOUT_VALUE           1000
#define BUFFER_SIZE                30        // Define o tamanho do payload aqui

char txpacket[BUFFER_SIZE];
static RadioEvents_t RadioEvents;

float txNumber;
bool lora_idle = true;

void setup() {
  Serial.begin(115200);
  txNumber = 0;

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
  if (lora_idle == true) {
    delay(1000);
    txNumber += 1;
    sprintf(txpacket, "nes %.0f", txNumber);
    Serial.printf("\r\nEnviando pacote \"%s\" , tamanho %d\r\n", txpacket, strlen(txpacket));
    turnOnRGB(COLOR_SEND, 0); // Muda a cor do RGB
    Radio.Send((uint8_t *)txpacket, strlen(txpacket));
    lora_idle = false;
  }
}

void OnTxDone(void) {
  turnOffRGB();
  Serial.println("Transmissão concluída.");
  lora_idle = true;
}

void OnTxTimeout(void) {
  turnOffRGB();
  Radio.Sleep();
  Serial.println("Tempo de transmissão excedido.");
  lora_idle = true;
}
