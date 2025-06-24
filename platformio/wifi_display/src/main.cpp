#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <../include/config.h>

// DISPLAY CONFIGURATION
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// FUNCOES
void connectWiFi();
void displayWeatherData();

void setup() {
  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha OLED no 0x3C.");
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println("Falha OLED no 0x3D.");
      while (true);
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Iniciando...");
  display.display();

  connectWiFi();
}

void loop() {
  if(WiFi.status() == WL_CONNECTED) {
    delay(30000);
    displayWeatherData();
  }
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");
  displayWeatherData();
}

void displayWeatherData() {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  String apiKey = API_KEY;
  String lat = LATITUDE;
  String lon = LONGITUDE;
  String url = "https://api.openweathermap.org/data/2.5/weather?lat=" + lat + "&lon=" + lon + "&appid=" + apiKey + "&units=metric";

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Payload: " + payload);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    const char* city = doc["name"];
    float temp = doc["main"]["temp"];
    const char* weather = doc["weather"][0]["description"];
    int humidity = doc["main"]["humidity"];

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(String("Cidade: ") + (city ? city : "N/A"));
    display.println(String("Temp: ") + String(temp, 1) + " C");
    display.println(String("Clima: ") + (weather ? weather : "N/A"));
    display.println(String("Umidade: ") + String(humidity) + "%");
    display.display();
  }
  http.end();
}