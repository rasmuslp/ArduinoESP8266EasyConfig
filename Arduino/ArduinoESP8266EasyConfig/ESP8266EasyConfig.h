#ifndef ESP8266EasyConfig_h
#define ESP8266EasyConfig_h

#include "Arduino.h"

#define BUFFER_SIZE 512

class ESP8266EasyConfig {
  public:
    ESP8266EasyConfig(Stream &serial);
    ~ESP8266EasyConfig();
    void begin(String ssid, String password = "");
    void end();
    void reset();

  private:
    Stream &_serial;
    char _readBuf[BUFFER_SIZE];
    byte _rbIndex = 0;
    boolean _serverIsRunning = false;
    String _ssid;
    String _password;

    boolean setupAP();
    String sendCommand(String command, String success, String failure, boolean transaction = false);
    String readCom();
    String findValue(String input, String key);
    String receiveData();
};

#endif
