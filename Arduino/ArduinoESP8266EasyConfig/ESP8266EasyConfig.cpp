#include "Arduino.h"

#include "ESP8266EasyConfig.h"

ESP8266EasyConfig::ESP8266EasyConfig(Stream &serial) : _serial(serial) {
  _serial.setTimeout(5000);
  _ackBuf[0] = '\0';
  _ackBuf[1] = '\0';
  _ackBuf[2] = '\0';
}

ESP8266EasyConfig::~ESP8266EasyConfig() {
  end();
}

void ESP8266EasyConfig::begin(String ssid, uint8_t channel, String password, uint8_t encryption) {
  Serial.println(F("ESP8266 EasyConfig beginning..."));
  
  _ssid = ssid;
  _channel = channel;
  _password = password;
  _encryption = encryption;

  while (_serial.available() > 0) {
    _serial.read();
  }

  // Reset module
  if (sendCmd("AT+RST", "OK")) {
    Serial.println(F("ESP8266 Reset OK"));
  } else {
    Serial.println(F("ESP8266 Reset failed"));
  }

  delay(500);
  while (_serial.available() > 0) {
    _serial.read();
  }

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

      //TODO
      readValue = readValue.substring(commandDelimiter + 2);

      // Parse commands
      if (command.equals("join")) {
        Serial.println(F("Got join command"));
        String ssid = findValue(readValue, "ssid");
        String pass = findValue(readValue, "pass");
        Serial.print("SSID: ");
        Serial.println(ssid);
        Serial.print("Password: ");
        Serial.println(pass);
        
        //TODO Save SSID and pass ?

        sendCommand("AT+CIPSERVER=0,8080");
        //      _serial.println();
        //      delay(500);
        //      if(_serial.find("OK")) {
        //        Serial.println("Yay");
        //      } else {
        //        Serial.println("Nay");
        //      }

        if (sendCmd("AT+CWMODE=1", "OK")) {
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
        if (sendCmd(_cmdBuf, "OK")) {
          Serial.println(F("ESP8266 Joined"));
          break;
        } else {
          Serial.println(F("ESP8266 Join failed"));
        }

      }


    }
  }
  
  Serial.println(F("ESP8266 EasyConfig done!"));
}

void ESP8266EasyConfig::end() {
  sendCommand("AT+CWQAP");
}

void ESP8266EasyConfig::reset() {
  end();
  begin(_ssid, _channel, _password, _encryption);
}

boolean ESP8266EasyConfig::setupAP() {
  if (sendCmd("AT+CIPMUX=1", "OK")) {
    Serial.println(F("ESP8266 Multiconnection mode enabled"));
  } else {
    Serial.println(F("ESP8266 Multiconnection mode failed"));
  }

  sendCommand("AT+CWMODE=2");


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

  if (sendCmd("AT+CIPSERVER=1,8080", "OK")) {
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

boolean ESP8266EasyConfig::sendCmd(char* command, char* successCriteria, boolean expectData) {
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

String ESP8266EasyConfig::findValue(String input, String key) {
  Serial.println("Got: " + input);

  String ret = "Key not found: " + key;
  int eqIndex = input.indexOf('=');
  while (eqIndex > -1) {
    String k = input.substring(0, eqIndex);
    Serial.println("Found key: " + k);
    String v;
    int ampIndex = input.indexOf('&');
    if (ampIndex > -1) {
      v = input.substring(eqIndex + 1, ampIndex);
      input = input.substring(ampIndex + 1);
    } else {
      v = input.substring(eqIndex + 1);
    }
    Serial.println("Found value: " + v);
    if (k.equals(key)) {
      ret = v;
      break;
    } else {
      eqIndex = input.indexOf('=');
    }
  }

  return ret;
}

String ESP8266EasyConfig::receiveData() {
  if (!_serial.find("+IPD,")) {
    return "";
  }

  String ret = _serial.readStringUntil('\r');
  boolean dataOk = true;

  int connectionIdDelimiter = ret.indexOf(',');
  if (connectionIdDelimiter == -1) {
    dataOk = false;
  }

  int startDelimiter = ret.indexOf(':');
  if (startDelimiter == -1) {
    dataOk = false;
  }

  if (dataOk) {
    Serial.print("Conn ID: ");
    Serial.println(ret.substring(0, connectionIdDelimiter));
    ret = ret.substring(startDelimiter + 1);
  } else {
    Serial.println(F("Data malformed?"));
    Serial.println(ret);
    ret = "";
  }

  return ret;
}

void ESP8266EasyConfig::chewCRLF() {
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

String ESP8266EasyConfig::sendCommand(String command, boolean transaction) {
  _serial.println(command);
  delay(1000);
  String ret = readCom();
  if (transaction) {
    Serial.println(F("Transaction:"));
    Serial.println(ret);
  }
  return ret;
}

String ESP8266EasyConfig::readCom() {
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
