#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --- WiFi設定 ---
const char* ssid = "";         // ←Wi-FiのSSIDを入力
const char* password = ""; // ←Wi-Fiのパスワードを入力

// --- Google Apps ScriptのWeb Apps URL（デプロイ時に取得）---
const char* scriptURL = "https://script.google.com/macros/s/AKfycbxrZB4MqCI6c4KG5JRhbs_PITHKkgFNSNC1hJkxvVKp1KvZ2FQI6cgsKm55yWROv7leHw/exec";

// --- I2C設定 ---
#define I2C_SLAVE_ADDR 8

struct SensorData {
  float temperature;
  float humidity;
  float pressure;
  float latitude;
  float longitude;
};

SensorData receivedData;

void receiveEvent(int bytes) {
  if (bytes == sizeof(SensorData)) {
    Wire.readBytes((char*)&receivedData, sizeof(SensorData));
  } else {
    while (Wire.available()) Wire.read(); 
  }
}

// Google SheetsにPOST送信
void sendToSheet(const SensorData& data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(scriptURL);
    http.addHeader("Content-Type", "application/json");
    String payload = String("{\"temperature\":") + data.temperature +
                     ",\"humidity\":" + data.humidity +
                     ",\"pressure\":" + data.pressure +
                     ",\"latitude\":" + data.latitude +
                     ",\"longitude\":" + data.longitude + "}";
    int httpResponseCode = http.POST(payload);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}

void setup() {
  Serial.begin(115200);
  // --- WiFi接続 ---
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // --- I2Cスレーブとして初期化 ---
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onReceive(receiveEvent);

  // 初期値表示
  Serial.println("Ready to receive I2C data and post to Google Sheets.");
}

void loop() {
  sendToSheet(receivedData);
  Serial.printf("T:%.2f H:%.2f P:%.2f Lat:%.6f Lon:%.6f\n",
    receivedData.temperature, receivedData.humidity, receivedData.pressure,
    receivedData.latitude, receivedData.longitude);
  delay(1000);
}
