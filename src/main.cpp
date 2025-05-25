#include <Arduino.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>

const char* ssid = "Pigeon";
WebServer server(80); // Port 80 http server

void handleRoot() { // At webpage http://198.162.4.1/

  File file = SPIFFS.open("/index.html", "r");
  if (!file){
    Serial.println("File reading failed");
  }
  server.streamFile(file, "text/html");
  file.close();

}

void handleSubmit(){
  //invoked when a form is submited
  Serial.println("invoked event");
}

void setup() {
  delay(5);
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

  server.on("/submit", handleSubmit);

  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
}