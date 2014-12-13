#include "Arduino.h"

#include "ESP8266EasyConfig.h"

#include "Utility.h"

#define DEBUG

#ifdef DEBUG
#define DBG(message) Serial.print(message)
#define DBGLN(message) Serial.println(message)
#else
#define DBG(message)
#define DBGLN(message)
#endif

ESP8266EasyConfig::ESP8266EasyConfig(Stream &serial) : _serial(serial) {
}

ESP8266EasyConfig::~ESP8266EasyConfig() {
  //TODO end();
}

bool ESP8266EasyConfig::begin(void) {
  _serial.flush();
  //TODO Timeout needed ?
  //_serial.setTimeout(30000);

  _modulePresent = reset();

  return _modulePresent;
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
        success = reset();
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
  
  println(F("AT+CWMODE?"));
  
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

  print(F("AT+CWMODE="));
  println(String(mode));

  String data = "";
  return readCmdResult(data);
}

bool ESP8266EasyConfig::listWifis(String &data) {
  if (!_modulePresent) {
    return false;
  }
  
  println(F("AT+CWLAP"));

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

  print(F("AT+CWJAP="));
  print(F("\""));
  print(ssid);
  print(F("\""));
  print(F(","));
  print(F("\""));
  print(password);
  println("\"");

  String data = "";
  return readCmdResult(data, 25000);
}

bool ESP8266EasyConfig::getAPInfo(String &ssid) {
  if (!_modulePresent) {
    return false;
  }

  println(F("AT+CWJAP?"));

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

  println(F("AT+CWQAP"));

  String data = "";
  return readCmdResult(data);
}

bool ESP8266EasyConfig::hostSoftAP(const String ssid, const String password, const uint8_t channel, const uint8_t encryption) {
  if (!_modulePresent) {
    return false;
  }

  print(F("AT+CWSAP="));
  print(F("\""));
  print(ssid);
  print(F("\""));
  print(F(","));
  print(F("\""));
  print(password);
  print("\"");
  print(F(","));
  print(String(channel));
  print(F(","));
  println(String(encryption));

  String data = "";
  return readCmdResult(data);
}

bool ESP8266EasyConfig::getSoftAPInfo(String &data) {
  if (!_modulePresent) {
    return false;
  }

  println(F("AT+CWSAP?"));

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

  println(F("AT+CWLIF"));

  if (!readCmdResult(data)) {
    return false;
  }
  data.replace(F("AT+CWLIF"), "");
  data.replace(F("+CWSAP?:"), "");
  data.replace(F("OK"), "");
  data.trim();
  
  return true;
}

bool ESP8266EasyConfig::reset(void) {
  //TODO? end();

  bool success = false;

  println(F("AT+RST"));
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

void ESP8266EasyConfig::easyConfig(String ssid, uint8_t channel, String password, uint8_t encryption) {
  Serial.println(F("ESP8266 EasyConfig beginning..."));

  _ssid = ssid;
  _channel = channel;
  _password = password;
  _encryption = encryption;

  // Reset module
  if (sendCmd("AT+RST", "OK")) {
    Serial.println(F("ESP8266 Reset OK"));
  } else {
    Serial.println(F("ESP8266 Reset failed"));
  }
  delay(500);
  _serial.find("ready");

  // Check device is OK
  if (sendCmd("AT", "OK")) {
    Serial.println(F("ESP8266 is found!"));
  } else {
    Serial.println(F("ESP8266 not found!"));
  }

  // Get firmware version
  if (sendCmd("AT+GMR", "OK", true)) {
    Serial.print(F("ESP8266 Got firmware version: "));
    Serial.println(_readBuf);
  } else {
    Serial.println(F("ESP8266 did not get firmware version"));
  }

  delay(5000);

  sendCommand("AT+CWSAP?", true);
  sendCommand("AT+CWJAP?", true);

  // Get CWMODE
  if (sendCmd("AT+CWMODE?", "OK", true)) {
    Serial.print(F("ESP8266 Is in CWMODE: "));
    Serial.println(_readBuf);
  } else {
    Serial.println(F("ESP8266 Could not get CWMODE"));
  }

  while (true) {
    if (!_serverIsRunning) {
      _serverIsRunning = setupAP();
    } else {
      String readValue = receiveData();
      if (readValue.equals("")) {
        continue;
      }

      // Verify request start and end
      int reqStart = readValue.indexOf("GET /");
      if (reqStart == -1) {
        // No request start, leaving...
        Serial.print(F("Invalid request: "));
        Serial.println(readValue);
        continue;
      }
      int reqEnd = readValue.lastIndexOf(" HTTP/");
      if (reqEnd == -1) {
        // No request end, leaving...
        Serial.print(F("Invalid request: "));
        Serial.println(readValue);
        continue;
      }

      readValue = readValue.substring(reqStart + 5, reqEnd);

      //TODO: Remove
      Serial.println(readValue);

      // Read command
      int commandDelimiter = readValue.indexOf('/');
      if (commandDelimiter == -1) {
        // No command delimiter, leaving...
        Serial.print(F("Invalid command: "));
        Serial.println(readValue);
        continue;
      }
      String command = readValue.substring(0, commandDelimiter);
      Serial.println(F("Command:"));
      Serial.println(command);

      // Starts after <command>/?
      readValue = readValue.substring(commandDelimiter + 2);

      // Parse commands
      if (command.equals("join")) {
        Serial.println(F("Got join command"));
        String ssid = Utility::findValue(readValue, "ssid");
        String pass = Utility::findValue(readValue, "pass");
        Serial.print("SSID: ");
        Serial.println(ssid);
        Serial.print("Password: ");
        Serial.println(pass);

        //TODO Save SSID and pass ?

        //sendCommand("AT+CIPSERVER=0,80");

        //        if (sendCmd("AT+CIPSERVER=0,80", "OK")) {
        //          Serial.println(F("ESP8266 Server is stopped"));
        //        } else {
        //          Serial.println(F("ESP8266 Server failed to stop"));
        //        }

        if (sendCmd("AT+CWMODE=3", "OK")) {
          Serial.println(F("ESP8266 Changed to STA mode"));
        } else {
          Serial.println(F("ESP8266 Change to STA mode failed"));
        }

        String joinCmd = "AT+CWJAP=\"";
        joinCmd += ssid;
        joinCmd += "\",\"";
        joinCmd += pass;
        joinCmd += "\"";
        joinCmd.toCharArray(_cmdBuf, CMD_BUF_SIZE);
        Serial.print(F("ESP8266 Joining SSID "));
        Serial.println(ssid);
        sendCommand(_cmdBuf);
        break;
        //        if (sendCmd(_cmdBuf, "OK")) {
        //          Serial.println(F("ESP8266 Joined"));
        //          break;
        //        } else {
        //          Serial.println(F("ESP8266 Join failed"));
        //        }

      }


    }

  }

  delay(2000);

  burnBuffer();
  //  if (sendCmd("AT+CIPSERVER=1,80", "OK")) {
  //    Serial.println(F("ESP8266 Server is running"));
  //  } else {
  //    Serial.println(F("ESP8266 Server initialization failed"));
  //  }
  burnBuffer();


  // Get IP
  sendCommand("AT+CIFSR");
  if (sendCmd("AT+CIFSR", "OK", true)) {
    Serial.print(F("ESP8266 Got IP: "));
    Serial.println(_readBuf);
  } else {
    Serial.println(F("ESP8266 Failed to get IP"));
  }

  Serial.println(F("ESP8266 EasyConfig done!"));
}

void ESP8266EasyConfig::end(void) {
  sendCommand("AT+CWQAP");
}

bool ESP8266EasyConfig::setupAP() {
  if (sendCmd("AT+CIPMUX=1", "OK")) {
    Serial.println(F("ESP8266 Multiconnection mode enabled"));
  } else {
    Serial.println(F("ESP8266 Multiconnection mode failed"));
  }

  sendCommand("AT+CWMODE=3");


  //  _serial.println("AT+CWMODE=2");
  //  delay(1000);
  //  if (_serial.find("OK"))
  //  {
  //    Serial.println(F("Acce_serial Point mode enabled"));
  //  }
  //  else
  //  {
  //    Serial.println(F("Acce_serial Point mode failed to initiate"));
  //
  //  }

  if (sendCmd("AT+CIPSERVER=1,80", "OK")) {
    Serial.println(F("ESP8266 Server is running"));
  } else {
    Serial.println(F("ESP8266 Server initialization failed"));
  }

  //Setup the AP with _serialID MORAS, on channel 11, with no pa_serialword

  String apCommand = "AT+CWSAP=\"";
  apCommand += _ssid;
  apCommand += "\",\"";
  apCommand += _password;
  apCommand += "\"";
  apCommand += _channel;
  apCommand += ",";
  apCommand += _encryption;
  apCommand.toCharArray(_cmdBuf, CMD_BUF_SIZE);
  if (sendCmd(_cmdBuf, "OK")) {
    Serial.println(F("ESP8266 Access point operational!"));
  } else {
    Serial.println(F("ESP8266 Access point initialization failed"));
  }

  return true;
}

bool ESP8266EasyConfig::sendCmd(char* command, char* successCriteria, bool expectData) {
  _serial.println(command);
  _serial.find(command);

  if (expectData) {
    chewCRLF();
    int readBytes = _serial.readBytesUntil('\r', _readBuf, READ_BUF_SIZE);
    _readBuf[readBytes] = '\0';
    if (readBytes > 0) {
      Serial.print("Read data: ");
      Serial.println(_readBuf);
    }
  }

  chewCRLF();
  _ackBuf[0] = _serial.read();
  _ackBuf[1] = _serial.read();

  //Serial.println("ACK:");
  //Serial.println(_ackBuf);

  if (strcmp(_ackBuf, successCriteria) == 0) {
    return true;
  }

  return false;
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
    Serial.println(idString);
    int idInt = idString.toInt();
    Serial.println(idInt);
    id = (int8_t) idInt;
    Serial.println(id);
    readData = readData.substring(startDelimiter + 1);
  } else {
    Serial.println(F("Data malformed?"));
    Serial.println(readData);
    readData = "";
  }

  burnBuffer();

  return readData;
}

String ESP8266EasyConfig::sendData(int8_t id, String data) {
  String sendCmd = "AT+CIPSEND=";
  sendCmd += id;
  sendCmd += ",";
  sendCmd += data.length();
  _serial.println(sendCmd);
  if (_serial.find(">")) {
    _serial.print(data);
  } else {
    _serial.print("AT+CIPCLOSE=");
    _serial.println(id);
  }
  sendCmd.toCharArray(_cmdBuf, CMD_BUF_SIZE);
  sendCommand(sendCmd, true);
  //  if (sendCmd(_cmdBuf, "OK")) {
  //    Serial.println(F("ESP8266 Send starting"));
  //  } else {
  //    Serial.println(F("ESP8266 Failed to start sending"));
  //  }
}

void ESP8266EasyConfig::burnBuffer(void) {
  while (_serial.available() > 0) {
    _serial.read();
  }
}

void ESP8266EasyConfig::chewCRLF(void) {
  while (true) {
    char c = _serial.peek();
    if (c == -1) {
      delay(10);
      continue;
    }
    if (c == '\r' || c == '\n') {
      _serial.read();
    } else {
      break;
    }
  }
}

void ESP8266EasyConfig::print(const __FlashStringHelper *fsh) {
  Serial.print(fsh);
  _serial.print(fsh);
}

void ESP8266EasyConfig::print(const String &s) {
  Serial.print(s);
  _serial.print(s);
}

void ESP8266EasyConfig::println(const __FlashStringHelper *fsh) {
  Serial.println(fsh);
  _serial.println(fsh);
}

void ESP8266EasyConfig::println(const String &s) {
  Serial.println(s);
  _serial.println(s);
}

String ESP8266EasyConfig::sendCommand(String command, bool transaction) {
  _serial.println(command);
  delay(1000);
  String ret = readCom();
  if (transaction) {
    Serial.println(F("Transaction:"));
    Serial.println(ret);
  }
  return ret;
}

String ESP8266EasyConfig::readCom(void) {
  String ret = "";
  byte _rbIndex = 0;

  while (_serial.available() > 0) {
    if (_rbIndex > (BUFFER_SIZE - 2)) {
      Serial.println(F("Read buffer full :<"));
      //return false;
    }
    char c = _serial.read();
    _readBuf[_rbIndex++] = c;
    _readBuf[_rbIndex] = '\0';
  }

  if (_rbIndex > 0) {
    ret = _readBuf;
    _readBuf[0] = '\0';
  }

  return ret;
}
