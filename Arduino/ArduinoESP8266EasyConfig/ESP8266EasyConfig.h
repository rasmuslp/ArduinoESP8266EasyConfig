#ifndef ESP8266EasyConfig_h
#define ESP8266EasyConfig_h

#include "Arduino.h"

#define BUFFER_SIZE 512

#define CMD_BUF_SIZE 64
#define READ_BUF_SIZE 128

class ESP8266EasyConfig {
  public:
    ESP8266EasyConfig(Stream &serial);
    ~ESP8266EasyConfig();
    void begin(String ssid, uint8_t channel = 11, String password = "", uint8_t encryption = 0);
    void end();
    void reset();

  private:
    Stream &_serial;
    String _ssid;
    uint8_t _channel;
    String _password;
    uint8_t _encryption;
    
    char _ackBuf[3];
    char _cmdBuf[CMD_BUF_SIZE];
    char _readBuf[READ_BUF_SIZE];
    
    boolean _serverIsRunning = false;

    boolean setupAP();
    boolean sendCmd(char* command, char* successCriteria, boolean expectData = false);
    
    String findValue(String input, String key);
    String receiveData();
    
    void chewCRLF();
    
    // Remove
    String sendCommand(String command, boolean transaction = true);
    String readCom();
};

#endif
