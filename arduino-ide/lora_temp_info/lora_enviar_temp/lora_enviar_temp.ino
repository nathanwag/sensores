#include "LoRaWan_APP.h"
#include "Arduino.h"

#include "BME680.h"

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

BME680_Class BME680;

static RadioEvents_t RadioEvents;

float altitude(const float seaLevel=1013.25) 
{
  static float Altitude;
  int32_t temp, hum, press, gas;
  BME680.getSensorData(temp,hum,press,gas);
  Altitude = 44330.0*(1.0-pow(((float)press/100.0)/seaLevel,0.1903));
  return(Altitude);
}

void setup() {
  Serial.begin(115200);

  // Inicializa BME680
  while (!BME680.begin(I2C_STANDARD_MODE))
  {
    Serial.println(F("-  Unable to find BME680. Waiting 3 seconds."));
    delay(3000);
  }

  // Configura BME680
  BME680.setOversampling(TemperatureSensor,Oversample16);
  BME680.setOversampling(HumiditySensor,   Oversample16);
  BME680.setOversampling(PressureSensor,   Oversample16);
  BME680.setIIRFilter(IIR4);
  BME680.setGas(320,150);

  // Inicializa rádio
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
  char txpacket[32];
  static int32_t temperature, humidity, pressure, gas;

  BME680.getSensorData(temperature, humidity, pressure, gas);

  // Monta a mensagem com todos os dados
  snprintf(txpacket, sizeof(txpacket),
           "%.1f %.1f %.0f %.1f %.1f",
           temperature / 100.0,
           humidity / 1000.0,
           pressure / 100.0,
           altitude(),
           gas / 100.0);

  Serial.printf("Enviando: \"%s\" (tamanho %d)\r\n", txpacket, strlen(txpacket));
  Radio.Send((uint8_t *)txpacket, strlen(txpacket));

  delay(10000);
}

void OnTxDone(void) {
  turnOnRGB(0x00FF00, 0);
  turnOffRGB();
}

void OnTxTimeout(void) {
  turnOnRGB(COLOR_SEND, 0);
  turnOffRGB();
  Radio.Sleep();
  Serial.println("Tempo de transmissão excedido.");
}

