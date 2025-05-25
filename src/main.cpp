#include <Arduino.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
bool isPressed = false;
String keyState = "None";
String espMode = "SoftAP";
bool prevPressed = false;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



void screenPrint(int x, int y, String text,bool debug = false){
  display.setCursor(x,y);
  display.println(text);
  display.display();
  if (debug){
    Serial.print("\n ");
    Serial.println(text);
  }

}



//==============//
//Website stuff//
//==============//
const char* ssid = "Pigeon";
WebServer server(80); // Port 80 http server

void handleRoot() { // At webpage http://192.168.4.1/

  File file = SPIFFS.open("/index.html", "r");
  if (!file){
    Serial.println("File reading failed");
  }
  server.streamFile(file, "text/html");
  file.close();

}

void handleSubmit() {
  String json = server.arg("plain");  // Raw body of POST request
  Serial.println("Received JSON: " + json);

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  String value = doc["data"];
  Serial.println("Received data: " + value);
  server.send(200, "text/plain", "pigeon recieved");

}


void setup() {
  Serial.begin(115200);
  Serial.println("starting");

  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(ssid)){
    Serial.println("Soft AP creation failed");
    return;
  }

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  
  server.on("/", handleRoot);
  server.begin();

  server.on("/submit",HTTP_POST, handleSubmit);

  Serial.println(WiFi.softAPIP());
  
  //OLED
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  pinMode(34,INPUT);
}

void loop() {
  server.handleClient();


  //OLED
  //Serial.println(digitalRead(34));
   if (digitalRead(34)){
    isPressed = true;
  }
  else{
    isPressed = false;
  }
  if (prevPressed == isPressed){
      keyState = "None";
    }
    else if (isPressed == false){
      keyState = "kUp";
    } 
    else if(isPressed == true){
      keyState = "kDown";
    }
  prevPressed = isPressed;
  
  
  if (keyState == "kDown"){
    Serial.println("You pressed a button!");
    if (espMode == "SoftAP"){
      espMode = "HardAP";
      Serial.println("Switched to HardAP!");
      display.clearDisplay();
    }
    else{
      espMode = "SoftAP";
      Serial.println("Switched to SoftAP!");
      display.clearDisplay();
    }
  }

  display.setCursor(0, 20);
  display.setTextColor(SSD1306_WHITE);
  // Serial.print("cursor has been set\n");
  // Serial.print("display cleared 1x\n");
  
  display.setTextSize(1);
  // Serial.print("text size set\n");
  display.println("PIGEON");

  display.display();
  // Serial.print("Drew pixels, display()'d as well.\n");
  screenPrint(0,0,"ESP32 State = " + espMode);

  
}