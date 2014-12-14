#include "ESP8266EasyConfig.h"

#include <SoftwareSerial.h>

#define SOFT_SERIAL_RX 7
#define SOFT_SERIAL_TX 8

#define RESET_BTN 6

//SoftwareSerial ss(SOFT_SERIAL_RX, SOFT_SERIAL_TX); // RX, TX
//ESP8266EasyConfig easyConfig(ss);

ESP8266EasyConfig wifi(Serial1);

long lastTimestamp = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("EasyConfig sketch starting..."));
  //  ss.begin(9600);
  Serial1.begin(9600);
  while (!Serial1);

  wifi.begin();

  //  wifi.initialize(AP_STA);
  //  String wifis;
  //  easyConfig.listWifis(wifis);
  //  Serial.println("WiFis: ");
  //  Serial.println(wifis);
  //  int8_t mode = wifi.getMode();
  //  Serial.print("Mode: ");
  //  Serial.println(mode);

  wifi.easyConfig();

  pinMode(RESET_BTN, INPUT);
  Serial.println(F("EasyConfig sketch started"));
}

void loop() {
  long timestamp = millis();
  Serial.print("Time: ");
  Serial.println(timestamp - lastTimestamp);
  lastTimestamp = timestamp;

  if (digitalRead(RESET_BTN)) {
    // Reset
    Serial.println(F("Reset button was pressed"));
    wifi.restart();
  }

  int8_t id = -1;
  String readData = wifi.receiveData(id);
  if (id > -1) {
    Serial.print(F("Got data. ID: "));
    Serial.print(id);
    Serial.print(F(" data: "));
    Serial.println(readData);
    //wifi.sendData(id, "Hello World!");
  }
}
