#include "Arduino.h"

#include "ESP8266EasyConfig.h"

#include "Utility.h"
#include "common.h"

ESP8266EasyConfig::ESP8266EasyConfig(Stream &serial) : _serial(serial) {
}

ESP8266EasyConfig::~ESP8266EasyConfig() {
  //TODO end();
}

bool ESP8266EasyConfig::begin(void) {
  _serial.flush();
  
  comln(F("AT"));
  _modulePresent = readCmdResult();

  return _modulePresent;
}

bool ESP8266EasyConfig::restart(void) {
  //TODO? end();

  bool success = false;
  comln(F("AT+RST"));
  unsigned long start = millis();
  while (millis() - start < 5000) {
    if (_serial.find("ready")) {
      success = true;
      break;
    }
  }

  if (success) {
    DBGLN(F("ESP reset and ready"));
  } else {
    DBGLN(F("ESP module not found"));
  }

  return success;
}

bool ESP8266EasyConfig::getVersionInfo(String &data) {
  if (!_modulePresent) {
    return false;
  }

  comln(F("AT+GMR"));
  if (!readCmdResult(data)) {
    return false;
  }
  
  data.replace(F("AT+GMR"), "");
  data.replace(F("OK"), "");
  data.trim();

  return true;
}

bool ESP8266EasyConfig::enterDeepSleep(unsigned int timeInMs) {
  if (!_modulePresent) {
    return false;
  }
  
  String cmd = F("AT+GSLP=");
  cmd += timeInMs;

  comln(cmd);
  String data = "";
  if (!readCmdResult(data)) {
    return false;
  }
  
  data.replace(cmd, "");
  data.replace(F("OK"), "");
  data.trim();
  
  //TODO: Compare returned time with requested time ?

  return true;
}

bool ESP8266EasyConfig::enableEcho(bool echo) {
  if (!_modulePresent) {
    return false;
  }
  
  com(F("ATE"));
  comln(echo ? "1" : "0");
  if (!readCmdResult()) {
    return false;
  }

  return true;
}

bool ESP8266EasyConfig::initialize(const uint8_t mode, const String ssid, const String password, const uint8_t channel, const uint8_t encryption) {
  if (!_modulePresent) {
    return false;
  }

  bool success = false;
  switch (mode) {
    case STA:
    case AP:
    case AP_STA: {
        // Change mode
        success = setMode(mode);
        if (!success) {
          return false;
        }

        // Reset module
        success = restart();
        if (!success) {
          return false;
        }

        break;
      }
    default:
      DBG(F("ESP (initialize) Unknown mode: "));
      DBGLN(mode);
      return false;
  }

  // Join AP
  if (mode == STA || mode == AP_STA) {
    success = joinAP(ssid, password);
    if (!success) {
      return false;
    }
  }

  // Host AP
  if (mode == AP || mode == AP_STA) {
    success = hostSoftAP(ssid, password, channel, encryption);
    if (!success) {
      return false;
    }
  }

  return success;
}

int8_t ESP8266EasyConfig::getMode() {
  if (!_modulePresent) {
    return false;
  }

  comln(F("AT+CWMODE?"));

  String data = "";
  if (!readCmdResult(data)) {
    return -1;
  }

  int8_t mode = -1;
  if (data.indexOf(String(STA)) != -1) {
    mode = STA;
  } else if (data.indexOf(String(AP)) != -1) {
    mode = AP;
  } else if (data.indexOf(String(AP_STA)) != -1) {
    mode = AP_STA;
  }

  return mode;
}

bool ESP8266EasyConfig::setMode(const uint8_t mode) {
  if (!_modulePresent) {
    return false;
  }

  com(F("AT+CWMODE="));
  comln(String(mode));

  return readCmdResult();
}

bool ESP8266EasyConfig::listWifis(String &data) {
  if (!_modulePresent) {
    return false;
  }

  comln(F("AT+CWLAP"));

  bool success = readCmdResult(data, 10000);
  if (!success) {
    DBG(F("ESP (listAP) Error: "));
    DBGLN(data);
    return false;
  }

  int8_t occurrences = Utility::findNoOccurrences(data, "+CWLAP:");
  DBG(F("Number of APs: "));
  DBGLN(occurrences);

  //TODO: Extract information about available APs
  data.replace(F("AT+CWLAP"), "");
  data.replace(F("+CWLAP:"), "WiFi ");
  data.replace(F("OK"), "");
  data.trim();

  return true;
}

bool ESP8266EasyConfig::joinAP(const String ssid, const String password) {
  if (!_modulePresent) {
    return false;
  }

  com(F("AT+CWJAP="));
  com(F("\""));
  com(ssid);
  com(F("\""));
  com(F(","));
  com(F("\""));
  com(password);
  comln("\"");

  return readCmdResult(25000);
}

bool ESP8266EasyConfig::getAPInfo(String &ssid) {
  if (!_modulePresent) {
    return false;
  }

  comln(F("AT+CWJAP?"));

  String data = "";
  if (!readCmdResult(data)) {
    return false;
  }
  data.replace(F("AT+CWJAP?"), "");
  data.replace(F("+CWJAP?:\""), "");
  data.replace(F("\""), "");
  data.replace(F("OK"), "");
  data.trim();
  ssid = data;

  return true;
}

bool ESP8266EasyConfig::leaveAP(void) {
  if (!_modulePresent) {
    return false;
  }

  comln(F("AT+CWQAP"));

  return readCmdResult();
}

bool ESP8266EasyConfig::hostSoftAP(const String ssid, const String password, const uint8_t channel, const uint8_t encryption) {
  if (!_modulePresent) {
    return false;
  }

  com(F("AT+CWSAP="));
  com(F("\""));
  com(ssid);
  com(F("\""));
  com(F(","));
  com(F("\""));
  com(password);
  com("\"");
  com(F(","));
  com(String(channel));
  com(F(","));
  comln(String(encryption));

  return readCmdResult();
}

bool ESP8266EasyConfig::getSoftAPInfo(String &data) {
  if (!_modulePresent) {
    return false;
  }

  comln(F("AT+CWSAP?"));

  if (!readCmdResult(data)) {
    return false;
  }
  data.replace(F("AT+CWSAP?"), "");
  data.replace(F("+CWSAP?:"), "");
  data.replace(F("OK"), "");
  data.trim();

  return true;
}

bool ESP8266EasyConfig::getSoftAPConnectionIPs(String &data) {
  if (!_modulePresent) {
    return false;
  }

  comln(F("AT+CWLIF"));

  if (!readCmdResult(data)) {
    return false;
  }
  data.replace(F("AT+CWLIF"), "");
  data.replace(F("+CWSAP?:"), "");
  data.replace(F("OK"), "");
  data.trim();

  return true;
}

bool ESP8266EasyConfig::readCmdResult(const uint16_t timeoutInMs) {
  String data = "";
  return readCmdResult(data, timeoutInMs);
}

bool ESP8266EasyConfig::readCmdResult(String &data, const uint16_t timeoutInMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutInMs) {
    if (_serial.available() > 0) {
      char c = _serial.read();
      data += c;
    }

    if (data.indexOf("OK") != -1 || data.indexOf("no change") != -1) {
      return true;
    }

    if (data.indexOf("ERROR") != -1 || data.indexOf("FAIL") != -1) {
      //TODO: Capture busy ?
      // FAIL will follow a 'busy'
      while (_serial.available() > 0) {
        char c = _serial.read();
        data += c;
      }

      DBGLN(F("ESP (readCmdResult) Error: "));
      DBGLN(data);
      return false;
    }
  }

  DBGLN(F("ESP (readCmdResult) Timed out. Data: "));
  DBGLN(data);

  return false;
}

bool ESP8266EasyConfig::easyConfig(const String ssid, const String password, const uint8_t channel, const uint8_t encryption) {
  DBGLN(F("ESP8266 EasyConfig beginning..."));

  _ssid = ssid;
  _password = password;
  _channel = channel;
  _encryption = encryption;

  while (true) {
    if (!_easyConfigServer) {
      if (!initialize(AP, _ssid, _password, _channel, _encryption)) {
        DBGLN(F("ESP (easyConfig) Could not start soft AP."));
        return false;
      }

      comln(F("AT+CIPMUX=1"));
      if (!readCmdResult()) {
        DBGLN(F("ESP (easyConfig) Could not enable multiconnection mode."));
        //TODO: Fail gracefully instead
        return false;
      }

      comln(F("AT+CIPSERVER=1,80"));
      if (!readCmdResult()) {
        DBGLN(F("ESP (easyConfig) Could not start server."));
        return false;
      }

      _easyConfigServer = true;
    } else {
      String readValue = receiveData();
      if (readValue.equals("")) {
        continue;
      }

      // Verify request start and end
      int reqStart = readValue.indexOf("GET /");
      if (reqStart == -1) {
        // No request start, leaving...
        DBG(F("Invalid request: "));
        DBGLN(readValue);
        continue;
      }
      int reqEnd = readValue.lastIndexOf(" HTTP/");
      if (reqEnd == -1) {
        // No request end, leaving...
        DBG(F("Invalid request: "));
        DBGLN(readValue);
        continue;
      }

      readValue = readValue.substring(reqStart + 5, reqEnd);

      // Read command
      int commandDelimiter = readValue.indexOf('/');
      if (commandDelimiter == -1) {
        // No command delimiter, leaving...
        DBG(F("Invalid command: "));
        DBGLN(readValue);
        continue;
      }
      String command = readValue.substring(0, commandDelimiter);
      DBG(F("Command:"));
      DBGLN(command);

      // Starts after <command>/?
      readValue = readValue.substring(commandDelimiter + 2);

      // Parse commands
      if (command.equals("join")) {
        DBGLN(F("Got join command"));
        String ssid = Utility::findValue(readValue, "ssid");
        String password = Utility::findValue(readValue, "pass");
        DBG("SSID: ");
        DBGLN(ssid);
        DBG("Password: ");
        DBGLN(password);

        //TODO: Save SSID and pass ?

        //TODO: Send ack

        //TODO: Close connection

        comln(F("AT+CIPSERVER=0,80"));
        if (!readCmdResult()) {
          DBGLN(F("ESP (easyConfig) Could not close server."));
          return false;
        }

        if (!initialize(STA, ssid, password)) {
          DBGLN(F("ESP (easyConfig) Could not join AP."));
          //TODO Tell 'interface' that requested this, that it failed.
          return false;
        }

        break;
      }
    }
  }

  _easyConfigServer = false;

  burnBuffer();

  comln(F("AT+CIFSR"));
  String data = "";
  if (!readCmdResult(data)) {
    DBGLN(F("ESP (easyConfig) Failed to read IP."));
  } else {
    DBGLN(F("Got IP data:"));
    DBGLN(data);
  }


  DBGLN(F("ESP8266 EasyConfig done!"));
}

void ESP8266EasyConfig::end(void) {
  //TODO: What shoul be closed ?
}

String ESP8266EasyConfig::receiveData(void) {
  int8_t id;
  return receiveData(id);
}

String ESP8266EasyConfig::receiveData(int8_t &id) {
  if (!_serial.find("+IPD,")) {
    return "";
  }

  String readData = _serial.readStringUntil('\r');
  bool dataOk = true;

  int connectionIdDelimiter = readData.indexOf(',');
  if (connectionIdDelimiter == -1) {
    dataOk = false;
  }

  int startDelimiter = readData.indexOf(':');
  if (startDelimiter == -1) {
    dataOk = false;
  }

  if (dataOk) {
    String idString = readData.substring(0, connectionIdDelimiter);
    int idInt = idString.toInt();
    id = (int8_t) idInt;
    readData = readData.substring(startDelimiter + 1);
  } else {
    DBGLN(F("Data malformed?"));
    DBGLN(readData);
    readData = "";
  }

  burnBuffer();

  return readData;
}

void ESP8266EasyConfig::burnBuffer(void) {
  while (_serial.available() > 0) {
    _serial.read();
  }
}

void ESP8266EasyConfig::com(const __FlashStringHelper *fsh) {
  Serial.print(fsh);
  _serial.print(fsh);
}

void ESP8266EasyConfig::com(const String &s) {
  Serial.print(s);
  _serial.print(s);
}

void ESP8266EasyConfig::comln(const __FlashStringHelper *fsh) {
  Serial.println(fsh);
  _serial.println(fsh);
}

void ESP8266EasyConfig::comln(const String &s) {
  Serial.println(s);
  _serial.println(s);
}
