#ifndef ESP8266EasyConfig_h
#define ESP8266EasyConfig_h

#include "Arduino.h"

// Encryption modes
#define NONE 0
#define WEP 1
#define WPA_PSK 2
#define WPA2_PSK 3
#define WPA_WPA2_PSK 4

// Server modes
#define OPEN 1
#define CLOSE 0

// Wifi modes
#define STA 1
#define AP 2
#define AP_STA 3

// Buffer sizes
#define BUFFER_SIZE 256
#define CMD_BUF_SIZE 64
#define READ_BUF_SIZE 64

struct LAP {
  uint8_t encryption;
  String ssid;
  int8_t signalStrength;
  String macAddress;
  uint8_t channel;
};

class ESP8266EasyConfig {
  public:
    ESP8266EasyConfig(Stream &serial);
    ~ESP8266EasyConfig();
    
    // Start communication and test module
    bool begin(void);
    
    // Wi-Fi commands
    bool initialize(const uint8_t mode = AP, const String ssid = "EasyConfig", const String password = "", const uint8_t channel = 11, const uint8_t encryption = NONE);
    
    int8_t getMode();
    bool setMode(const uint8_t mode);
    
    // NB: Only works if in STA or AP_STA mode, will throw error otherwise
    bool listWifis(String &data);
    
    bool joinAP(const String ssid, const String password);
    bool getAPInfo(String &ssid);
    bool leaveAP(void);
    
    bool hostSoftAP(const String ssid, const String password, const uint8_t channel, const uint8_t encryption);
    bool getSoftAPInfo(String &data);
    bool getSoftAPConnectionIPs(String &data);
    
    bool reset(void);

    
    void easyConfig(String ssid, uint8_t channel = 11, String password = "", uint8_t encryption = 0);
    void end(void);

    
    String receiveData(void);
    String receiveData(int8_t &id);
    String sendData(int8_t id, String data);
   
  private:
    Stream &_serial;
    bool _modulePresent = false;
    String _ssid;
    uint8_t _channel;
    String _password;
    uint8_t _encryption;
    
    char _ackBuf[3] = {'\0', '\0', '\0'};
    char _cmdBuf[CMD_BUF_SIZE];
    char _readBuf[READ_BUF_SIZE];
    
    bool _serverIsRunning = false;

    bool setupAP(void);
    bool sendCmd(char* command, char* successCriteria, bool expectData = false);
    
    // NEW
    bool readCmdResult(String &data, const uint16_t timeoutInMs = 5000); 

    // Serial modification
    void burnBuffer(void);    
    void chewCRLF(void);
    void print(const __FlashStringHelper *fsh);
    void print(const String &s);
    void println(const __FlashStringHelper *fsh);
    void println(const String &s);
    
    // Remove
    String sendCommand(String command, bool transaction = true);
    String readCom(void);
};

#endif
