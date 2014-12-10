#include "ESP8266EasyConfig.h"

#include <SoftwareSerial.h>

#define SOFT_SERIAL_RX 7
#define SOFT_SERIAL_TX 8

#define LED_RED 3
#define LED_GREEN 4
#define LED_BLUE 5

#define RESET_BTN 6

SoftwareSerial ss(SOFT_SERIAL_RX, SOFT_SERIAL_TX); // RX, TX
ESP8266EasyConfig easyConfig(ss);

long lastTimestamp = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("EasyConfig sketch starting..."));
  ss.begin(9600);
  easyConfig.begin("MORAS");

  pinMode(RESET_BTN, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  Serial.println(F("EasyConfig sketch starting done"));
}

void loop() {
  long timestamp = millis();
  Serial.print("Time: ");
  Serial.println(timestamp - lastTimestamp);
  lastTimestamp = timestamp;
  
  if (digitalRead(RESET_BTN)) {
    // Reset
    Serial.println(F("Reset button was pressed"));
    easyConfig.reset();
  }
  
  uint8_t id = -1;
  String readData = easyConfig.receiveData(id);
  if (id > -1) {
    Serial.print(F("Got data. ID: "));
    Serial.print(id);
    Serial.print(F(" data: "));
    Serial.println(readData);
    easyConfig.sendData(id, "ROFL");
  }
}

