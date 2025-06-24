#include <Arduino.h>
#include "GPS_Air530Z.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include <math.h>

SSD1306Wire display(0x3c, 500000, SDA, SCL, GEOMETRY_128_64, GPIO10);
Air530ZClass GPS;

void VextON(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
}

void setup() {
  VextON();
  delay(10);

  display.init();
  display.clear();
  display.display();

  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 32 - 16 / 2, "GPS initing...");
  display.display();

  Serial.begin(115200);
  GPS.begin();
}

void loop() {
  uint32_t starttime = millis();
  while ((millis() - starttime) < 1000) {
    while (GPS.available() > 0) {
      GPS.encode(GPS.read());
    }
  }

  char str[30];
  char latStr[15], lonStr[15], altStr[15], hdopStr[15], speedStr[15];

  // Formatar os valores como strings corretamente
  dtostrf(GPS.location.lat(), 9, 6, latStr);
  dtostrf(GPS.location.lng(), 9, 6, lonStr);
  dtostrf(GPS.altitude.meters(), 7, 2, altStr);
  dtostrf(GPS.hdop.hdop(), 5, 2, hdopStr);
  dtostrf(GPS.speed.kmph(), 7, 3, speedStr);

  // Atualizar display
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

  // Enviar dados para a serial
  Serial.println("=== Dados do GPS ===");
  Serial.printf("Data: %02d-%02d-%02d\n", GPS.date.year(), GPS.date.day(), GPS.date.month());
  Serial.printf("Hora: %02d:%02d:%02d\n", GPS.time.hour(), GPS.time.minute(), GPS.time.second());
  Serial.printf("Latitude: %s\n", latStr);
  Serial.printf("Longitude: %s\n", lonStr);
  Serial.printf("Altitude: %s m\n", altStr);
  Serial.printf("HDOP: %s\n", hdopStr);
  Serial.printf("Velocidade: %s km/h\n", speedStr);
  Serial.println("====================\n");
  Serial.flush();  // Aguarda o envio dos dados

  // Aguarda confirmação do Python com timeout de 5 segundos
  unsigned long waitStart = millis();
  while (Serial.available() == 0) {
    if (millis() - waitStart > 5000) {
      Serial.println("Timeout waiting for confirmation");
      break;
    }
    delay(10);
  }
  if (Serial.available() > 0) {
    char confirmation = Serial.read();
    if (confirmation != 'A') {
      Serial.println("Invalid confirmation received");
    }
  }
}