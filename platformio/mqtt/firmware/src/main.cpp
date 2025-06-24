#include <Arduino.h>
#include <Wire.h>
#include "BMP085.h"
#include "BH1750.h"

/******************** Begin Settings ********************/

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
float currentTurbidity = 0;
void readTurbidity();

/***************************
 * Begin PH Sensor Settings
 **************************/ 
#define PH_PIN 39
int currentPH = 0;
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
}

void loop() {
  if (millis() - readTime > 10000) {
    readTemperatureHumidity();
    readLightIntensity();
    readAtmosphericPressure();
    readTurbidity();
    //readPH();
    readTime = millis();
  }
}

// Read data
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
  Serial.print(",Humidity:");
  Serial.print(currentHumidity);
}

void readLightIntensity() {
  uint16_t lux = lightSensor.readLightLevel();
  currentlightIntensity = lux;
  Serial.print(",LightIntensity:");
  Serial.print(currentlightIntensity);
}

void readAtmosphericPressure() {
  currentAtmosphericPressure = bmpSensor.readPressure();
  Serial.print(",AtmosphericPressure:");
  Serial.print(currentAtmosphericPressure);
}

void readTurbidity() {
  int sensorValue = analogRead(ANALOG_PIN);
  float voltage = sensorValue * (3.3 / 4095);
  currentTurbidity = ( (1.68 - voltage) / 1.68 ) * 200;

  if (currentTurbidity < 0) currentTurbidity = 0;
  else if (currentTurbidity > 200) currentTurbidity = 200;

  Serial.print(",Turbidez:");
  Serial.println(currentTurbidity, 1);
}

void readPH() {
  int sensorValue = analogRead(PH_PIN);
  float voltage_reduced = sensorValue * (3.3 / 4095.0);
  float voltage_original = voltage_reduced / 0.6226;
  float currentPH = 7.0 - ((voltage_original - 2.5) / 0.17);

  // Exibir no monitor serial
  Serial.print("pH Analógico: ");
  Serial.print(sensorValue);
  Serial.print(" | Vout (reduzida): ");
  Serial.print(voltage_reduced, 3);
  Serial.print(" V | Vout (original): ");
  Serial.print(voltage_original, 3);
  Serial.print(" V | pH: ");
  Serial.println(currentPH, 2);
}