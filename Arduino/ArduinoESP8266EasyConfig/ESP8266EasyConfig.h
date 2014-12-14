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

    /// Module commands
    bool begin(void);
    bool restart(void);
    bool getVersionInfo(String &data);
    bool enterDeepSleep(unsigned int timeInMs);
    bool enableEcho(const bool echo);

    /// Wi-Fi commands
    bool setMode(const uint8_t mode);
    int8_t getMode();
    bool enableDHCPFor(const uint8_t mode, const bool dhcp);

    // STA-mode
    bool connectToAP(const String ssid, const String password);
    bool getConnectedAPInfo(String &ssid);
    bool setSTAMac(const String mac);
    bool getSTAMac(String &data);
    bool setSTAIP(const String ip);
    bool getSTAIP(String &data);

    // NB: Only works if in STA or AP_STA mode, will throw error otherwise
    bool listWifis(String &data);
    bool searchForWifi(const String ssid, const String mac, const uint8_t channel, String &data);

    bool disconnectFromAP(void);

    // AP-mode
    bool startSoftAP(const String ssid, const String password, const uint8_t channel, const uint8_t encryption);
    bool getSoftAPInfo(String &data);
    bool getSoftAPClientIPs(String &data);
    bool setSoftAPMac(const String mac);
    bool getSoftAPMac(String &data);
    bool setSoftAPIP(const String ip);
    bool getSoftAPIP(String &data);

    // TCP/IP commands
    String receiveData(void);
    String receiveData(int8_t &id);

    /// EasyConfig
    bool initialize(const uint8_t mode = AP, const String ssid = "EasyConfig", const String password = "", const uint8_t channel = 11, const uint8_t encryption = NONE);
    bool easyConfig(const String ssid = "EasyConfig", const String password = "", const uint8_t channel = 11, const uint8_t encryption = NONE);

    // Old commands
    void end(void);

  private:
    Stream &_serial;
    bool _modulePresent = false;

    String _ssid;
    String _password;
    uint8_t _channel;
    uint8_t _encryption;

    bool _easyConfigServer = false;

    bool readCmdResult(const uint16_t timeoutInMs = 5000);
    bool readCmdResult(String &data, const uint16_t timeoutInMs = 5000);

    // Serial modification
    void burnBuffer(void);
    void com(const __FlashStringHelper *fsh);
    void com(const String &s);
    void comln(const __FlashStringHelper *fsh);
    void comln(const String &s);
};

#endif
