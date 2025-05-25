#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==== OLED Setup ====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==== WiFi and Server Setup ====
const char* ssid = "Pigeon";
const char* password = "william is cool";
WebServer server(80);
WebSocketsServer webSocket(81);

// ==== State Variables ====
bool isPressed = false;
bool prevPressed = false;
String keyState = "None";
String espMode = "SoftAP";

// ==== Utility Function ====
void screenPrint(int x, int y, const String& text, bool debug = false) {
  display.setCursor(x, y);
  display.println(text);
  display.display();
  if (debug) {
    Serial.println(text);
  }
}

// ==== Handle root index.html ====
void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    Serial.println("Failed to open index.html");
    server.send(500, "text/plain", "File Not Found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

// ==== Handle POST to /submit ====
void handleSubmit() {
  String json = server.arg("plain");
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  String message = doc["data"];
  IPAddress senderIP = server.client().remoteIP();
  Serial.println("[" + senderIP.toString() + "]: " + message);

  webSocket.broadcastTXT("[" + senderIP.toString() + "]: " + message);
  server.send(200, "text/plain", "Pigeon received");
}

// ==== WebSocket Event Handling ====
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("Client %u connected from %s\n", num, ip.toString().c_str());
      break;
    }
    case WStype_DISCONNECTED:

      Serial.printf("Client %u disconnected\n", num);
      break;
    case WStype_TEXT:
      Serial.printf("Received message from %u: %s\n", num, payload);
      break;
    default:
      break;
  }
}

// ==== Setup Function ====
void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  // OLED Init
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Button pin
  pinMode(34, INPUT);

  // SPIFFS Init
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  // WiFi Init
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(ssid,password)) {
    Serial.println("SoftAP Failed");
    return;
  }
  Serial.println("SoftAP IP: " + WiFi.softAPIP().toString());

  // HTTP Server Routes
  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.begin();

  // WebSocket Init
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

// ==== Main Loop ====
void loop() {
  server.handleClient();
  webSocket.loop();

  // Button State Handling
  isPressed = digitalRead(34);
  if (prevPressed != isPressed) {
    keyState = isPressed ? "kDown" : "kUp";
    prevPressed = isPressed;

    if (keyState == "kDown") {
      espMode = (espMode == "SoftAP") ? "HardAP" : "SoftAP";
      display.clearDisplay();
    }
  }

  display.clearDisplay();
  screenPrint(0, 0, "ESP32 State = " + espMode);
  display.setCursor(0, 20);
  display.println("PIGEON");
  display.display();
}
