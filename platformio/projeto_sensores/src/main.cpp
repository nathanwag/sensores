#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <Wire.h>
#include "BMP085.h"
#include "BH1750.h"
#include <../include/config.h>

/******************** Begin Settings ********************/

/***************************
 * WIFI Settings
 **************************/
const char* WiFi_Name = WIFI_SSID;
const char* WiFi_Password = WIFI_PASSWORD;
WiFiClient client;

/***************************
 * Thingspeak Settings
 **************************/
const char *thingspeak_Host = "api.thingspeak.com";
const char *thingspeak_API_key = API_KEY;
const int httpPort = 80;

/***************************
 * Begin DHT11 Settings
 **************************/
#define DHT11_Data_Pin 15
int currentTemperature = 0;
int currentHumidity = 0;
void readTemperatureHumidity();
void uploadDatatoThinkspeak();
long readTime = 0; 
long uploadTime = 0; 

/***************************
 * Begin BMP180 Atmosphere Sensor Settings
 **************************/
#define SDA_BH1750_BMP180_Pin 21
#define SCL_BH1750_BMP180_Pin 22
Adafruit_BMP085 bmpSensor;
int currentAtmosphericPressure = 0;
void readAtmosphericPressure();

/***************************
 * Begin BH1750FVI Light Sensor Settings
 **************************/
BH1750 lightSensor;
int currentlightIntensity = 0;
void readLightIntensity();

/***************************
 * Begin TS-300B Turbidity Settings
 **************************/
#define ANALOG_PIN 36
void readTurbidity();

/***************************
 * Begin PH Sensor Settings
 **************************/
#define PH_PIN 39
void readPH();

/******************** End Settings ********************/

void setup() {
  Serial.begin(115200);
  // Initialize BMP180 and BH1750FVI sensor
  Wire.begin(SDA_BH1750_BMP180_Pin, SCL_BH1750_BMP180_Pin);
  lightSensor.begin();
  if (!bmpSensor.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    while (1) {}
  } else {
    Serial.println("Find BMP180 Sensor");
  }

  WiFi.begin(WiFi_Name, WiFi_Password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  client.setTimeout(300000);
}

void loop() {
  // Read Temperature Humidity every 10 seconds
  if (millis() - readTime > 10000) {
    readTemperatureHumidity();
    readLightIntensity();
    readAtmosphericPressure();
    readTurbidity();
    readPH();
    readTime = millis();
  }
  // Upload Temperature Humidity every 120 seconds
  if (millis() - uploadTime > 120000) {
    Serial.println("---Upload Temperature Humidity every 120 seconds---");
    uploadDatatoThinkspeak();
    uploadTime = millis();
  }
}

// Read temperature humidity data
void readTemperatureHumidity() {
  unsigned int loopCnt;
  int chr[40] = {0};
  unsigned long time1;
bgn:
  delay(2000);
  // Set interface mode 2 to: output
  // Output low level 20ms (>18ms)
  // Output high level 40μs
  pinMode(DHT11_Data_Pin, OUTPUT);
  digitalWrite(DHT11_Data_Pin, LOW);
  delay(20);
  digitalWrite(DHT11_Data_Pin, HIGH);
  delayMicroseconds(40);
  digitalWrite(DHT11_Data_Pin, LOW);
  // Set interface mode 2: input
  pinMode(DHT11_Data_Pin, INPUT);
  // High level response signal
  loopCnt = 10000;
  while (digitalRead(DHT11_Data_Pin) != HIGH) {
    if (loopCnt-- == 0) {
      // If don't return to high level for a long time, output a prompt and start over
      Serial.println("HIGH");
      goto bgn;
    }
  }
  // Low level response signal
  loopCnt = 30000;
  while (digitalRead(DHT11_Data_Pin) != LOW) {
    if (loopCnt-- == 0) {
      // If don't return low for a long time, output a prompt and start over
      Serial.println("LOW");
      goto bgn;
    }
  }
  // Start reading the value of bit1-40
  for (int i = 0; i < 40; i++) {
    while (digitalRead(DHT11_Data_Pin) == LOW) {}
    // When the high level occurs, write down the time "time"
    time1 = micros();
    while (digitalRead(DHT11_Data_Pin) == HIGH) {}
    // When there is a low level, write down the time and subtract the time just saved
    // If the value obtained is greater than 50μs, it is ‘1’, otherwise it is ‘0’
    // And save it in an array
    if (micros() - time1 > 50) {
      chr[i] = 1;
    } else {
      chr[i] = 0;
    }
  }

  // Humidity, 8-bit bit, converted to a value
  currentHumidity = chr[0] * 128 + chr[1] * 64 + chr[2] * 32 + chr[3] * 16 + chr[4] * 8 + chr[5] * 4 + chr[6] * 2 + chr[7];
  // Temperature, 8-bit bit, converted to a value
  currentTemperature = chr[16] * 128 + chr[17] * 64 + chr[18] * 32 + chr[19] * 16 + chr[20] * 8 + chr[21] * 4 + chr[22] * 2 + chr[23];

  Serial.print("Temperature:");
  Serial.print(currentTemperature);
  Serial.print("    Humidity:");
  Serial.println(currentHumidity);
}

void readLightIntensity() {
  uint16_t lux = lightSensor.readLightLevel();
  currentlightIntensity = lux;
  Serial.print("Light Intensity: ");
  Serial.print(currentlightIntensity);
  Serial.println(" lx");
}

void readAtmosphericPressure() {
  currentAtmosphericPressure = bmpSensor.readPressure();
  Serial.print("Atmospheric Pressure = ");
  Serial.print(currentAtmosphericPressure);
  Serial.println(" Pa");
}

void readTurbidity() {
  // Ler saída analógica (0-4095, ADC de 12 bits)
  int sensorValue = analogRead(ANALOG_PIN);
  // Converter para voltagem no pino (0-3,3V)
  float vout = sensorValue * (3.3 / 4095.0);
  // Calcular tensão real do sensor (R1=10kΩ, R2=20kΩ)
  float vin = vout * (30.0 / 20.0); // (R1 + R2) / R2 = 30/20 = 1,5

  // Exibir no monitor serial
  Serial.print("Valor analógico: ");
  Serial.print(sensorValue);
  Serial.print(" | Vout: ");
  Serial.print(vout);
  Serial.print(" V | Vin: ");
  Serial.print(vin);
  Serial.println(" V");
}

void readPH() {
  // Ler valor bruto do ADC (pH)
  int sensorValue = analogRead(PH_PIN);
  // Converter para tensão (0-3.3V, ADC de 12 bits)
  float vout = sensorValue * (3.3 / 4095.0);
  // Calcular pH (ajuste conforme calibração)
  float phValue = 7.0 - ((vout - 2.5) / 0.059); // Aproximado: 59mV por pH

  // Exibir no monitor serial
  Serial.print("pH Analógico: ");
  Serial.print(sensorValue);
  Serial.print(" | Vout: ");
  Serial.print(vout, 3);
  Serial.print(" V | pH: ");
  Serial.println(phValue, 2);
}

// Upload temperature humidity data to thingspeak.com
void uploadDatatoThinkspeak() {
  if (!client.connect(thingspeak_Host, httpPort)) {
    Serial.println("connection Thingspeak failed");
    return;
  }
  // Three values(field1 field2 field3 field4) have been set in thingspeak.com 
  client.print(String("GET ") + "/update?api_key=" + thingspeak_API_key + "&field1=" + currentTemperature + "&field2=" + currentHumidity + "&field3=" + currentlightIntensity + "&field4=" + currentAtmosphericPressure + " HTTP/1.1\r\n" + "Host: " + thingspeak_Host + "\r\n" + "Connection: close\r\n\r\n");
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}