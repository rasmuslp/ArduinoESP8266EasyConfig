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

String ssid;
String pass;

void setup() {
  Serial.begin(9600);
  Serial.println("ESP8266 AP setup");
  ss.begin(9600);
  easyConfig.begin("MORAS");

  pinMode(RESET_BTN, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
}

void loop() {
  if (digitalRead(RESET_BTN)) {
    // Reset
    easyConfig.reset();
  }
}

