#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"

// WiFi & API
const char* ssid = "arrasy-family";
const char* password = "n@biel0418";
const char* datamallAccountKey = "Lfr5QCrfQ/OuLYsexjuEBw==";
const char* busStopCode = "20251";

#define BUTTON_PIN D5
unsigned long lastButtonPress = 0;
bool oledAwake = false;

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Timer
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 60000; // 1 menit

// --- Forward Declaration ---
void displayMessage(const char* message);
void getBusDataAndDisplay();

void setup() {
  Serial.begin(115200);

pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Init OLED
  Wire.begin(D2, D1);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

// Awal OLED redup
  display.dim(true);

  displayMessage("Connecting WiFi...");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    displayMessage("WiFi Connected");

    // Sinkronisasi waktu
    configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("Syncing time...");
    time_t now = time(nullptr);
    while (now < 1000000000) {
      delay(500);
      now = time(nullptr);
    }
    Serial.println("Time synced!");

    // Ambil data pertama kali
    getBusDataAndDisplay();

    // Set WiFi Light Sleep Mode
    wifi_set_sleep_type(LIGHT_SLEEP_T);
  } else {
    displayMessage("WiFi Failed!");
  }
}

void loop() {

// cek tombol ditekan
  if (digitalRead(BUTTON_PIN) == LOW) {
    lastButtonPress = millis();
    oledAwake = true;
    display.dim(false);  // nyalakan terang
    Serial.println("OLED Wakeup by Button!");
    delay(200); // debounce
  }

  // auto redup setelah 10 detik tanpa tombol
  if (oledAwake && (millis() - lastButtonPress > 15000)) {
    oledAwake = false;
    display.dim(true);   // redupkan lagi
    Serial.println("OLED Dim again");
  }

     // refresh JSON tiap 1 menit (tetap berjalan)
  static unsigned long lastRefresh = 0;
  if (millis() - lastRefresh > 60000) {
    lastRefresh = millis();
    getBusDataAndDisplay();
     display.dim(false);
    Serial.println("OLED Wakeup by Update!");
    delay(15000);
    if (!oledAwake) {   // hanya redup lagi jika bukan karena tombol
      display.dim(true);
      Serial.println("OLED Dim after Update");
    }
  }

  // hemat daya: Light Sleep WiFi
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
}


// === Ambil data JSON & tampilkan ke OLED ===
void getBusDataAndDisplay() {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = "https://datamall2.mytransport.sg/ltaodataservice/v3/BusArrival?BusStopCode=" + String(busStopCode);

  Serial.println("Requesting: " + url);

  http.begin(client, url);
  http.addHeader("AccountKey", datamallAccountKey);
  http.addHeader("accept", "application/json");

  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Response OK (truncated):");
    Serial.println(payload.substring(0, 200));

    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.println("JSON Parsing Error: ");
      Serial.println(error.c_str());
      displayMessage("JSON Error!");
      return;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.drawLine(0, 9, SCREEN_WIDTH, 9, WHITE);
    display.setCursor(10, 0);
    display.println("Bus Arrival Info:");
    display.drawLine(0, 9, SCREEN_WIDTH, 9, WHITE);

    JsonArray services = doc["Services"].as<JsonArray>();
    
    int y = 20;
    for (JsonObject service : services) {
      String serviceNo = service["ServiceNo"].as<String>();
      String nextBusETA_str = service["NextBus"]["EstimatedArrival"].as<String>();
      
      if (y > SCREEN_HEIGHT - 10) break;

      display.setCursor(0, y);
      display.print("Bus ");
      display.print(serviceNo);
      display.setCursor(60, y);
      display.print(": ");
      
      if (nextBusETA_str.length() > 0) {
        struct tm tm;
        strptime(nextBusETA_str.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
        time_t arrivalTime = mktime(&tm);
        time_t currentTime = time(nullptr);
        long minutes = (arrivalTime - currentTime) / 60;
        
        if (minutes <= 0) {
          display.print("Arr");
        } else {
          display.print(minutes);
          display.print(" min");
        }
      } else {
        display.print("N/A");
      }
      y += 10;
    }
    display.display();
  } else {
    String errorMsg = "HTTP Error: " + String(httpCode);
    Serial.println(errorMsg);
    displayMessage(errorMsg.c_str());
  }
  http.end();
}

void displayMessage(const char* message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(message);
  display.display();
  Serial.println(message);
}
