#include <ESP8266WiFi.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

char * resetSerialLed = "##03FF00";

int resetButton = 2; //D4 na NodeMCU
unsigned long resetStartTime;
int resetButtonHold = HIGH;

WiFiManager wifiManager;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void setup() {
    Serial.begin(19200);
    pinMode(resetButton, INPUT);
    Serial.println("AutoConnectStart");
    wifiManager.autoConnect("ElmesConf");
    Serial.println("connected...yeey :)");

    httpServer.on("/", handleRoot);
    httpServer.on("/send", handleSend);
    httpUpdater.setup(&httpServer);
    httpServer.begin();
}

void loop() {
  if(WiFi.status() != WL_CONNECTED) return;
  resetButtonHandler();
  httpServer.handleClient();
}

void resetButtonHandler() {
  int resetState = digitalRead(resetButton);
  if (resetState == HIGH) {
    resetButtonHold = HIGH;
  }
  if (resetButtonHold == HIGH && resetState == LOW) {
    resetStartTime = millis();
    resetButtonHold = LOW;
  }
  if (resetButtonHold == LOW && resetState == LOW) {
    unsigned long currentTime = millis();
    if (currentTime - resetStartTime > 5000) {
      resetButtonHold = HIGH;
      //resetuje dane wifi, zeby po restarcie przejsc w tryb AP
      WiFi.disconnect();
      Serial.println(resetSerialLed);
      delay(1000);
      ESP.restart();
    }
  }
}

void handleRoot() {
  String response = String("/update\n/send\n\nWersja 0.9");
  httpServer.send(200, "text/plain", response);
}

void handleSend() {
  String response;
  Serial.print("ARGS:");
  Serial.println(httpServer.args());
  if(httpServer.args() == 2 && httpServer.hasArg("channel") && httpServer.hasArg("button")) {
    char charBuf[50];
    int channel;
    int button;
    if (httpServer.hasArg("channel")) {
      httpServer.arg("channel").toCharArray(charBuf, 50);
      channel = atoi(charBuf);
    }
    if (httpServer.hasArg("button")) {
      httpServer.arg("button").toCharArray(charBuf, 50);
      button = atoi(charBuf);
    }
    
    response = String("Wyslano sygnal na kanal: ") + String(channel) + ", na przycisk: " + String(button);
    Serial.println(message(channel, button));
  } else {
    response = "/send?channel=AA&button=BB";
  }
  Serial.println(response);
  httpServer.send(200, "text/plain", response);
}

String message(int channel, int button) {
  if (channel >= 1 && channel <= 30) {
    return "Kom##03" + channelNumber(channel - 1) + "0" + String(button - 1);
  } else if (channel >= 31 && channel <= 34) {
    return "Kom##05" + channelNumber(channel - 1) + "0" + String(button - 1) + "0A00";
  }
}

String channelNumber(int channel) {
  String out = "";
  if (channel < 16) {
    out = "0" + String(channel,  HEX);
  } else {
    out = String(channel, HEX);
  }
  out.toUpperCase();
  return out;
}
