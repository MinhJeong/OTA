#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "OTAWebUpdater.h"

const char* host = "esp32";
const char* ssid = "3508B 2.4G";      
const char* password = "12345668";     

// Định nghĩa chân và biến cho LED
const int LED_PIN = 2;
unsigned long previousMillis = 0;
const long interval = 1000;  // Chu kỳ nhấp nháy 1 giây
bool ledState = false;

WebServer server(80);
OTAWebUpdater otaUpdater(&server, host);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Khởi tạo mDNS
  if (MDNS.begin(host)) {
    Serial.println("mDNS responder started");
    Serial.print("Bạn có thể kết nối qua http://");
    Serial.print(host);
    Serial.println(".local");
  }

  // Thiết lập OTA với username và password mặc định (admin/admin)
  otaUpdater.setup();
  
  // Đăng ký các endpoint cho OTA
  otaUpdater.registerEndpoints();

  // Cấu hình chân LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  
  // Xử lý nhấp nháy LED
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
  
  delay(1);
}
