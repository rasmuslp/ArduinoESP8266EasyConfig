#include "Arduino.h"

#include "ESP8266EasyConfig.h"

ESP8266EasyConfig::ESP8266EasyConfig(Stream &serial) : _serial(serial) {
  _serial = serial;
  _serial.setTimeout(5000);
}

ESP8266EasyConfig::~ESP8266EasyConfig() {
  end();
}

void ESP8266EasyConfig::begin(String ssid, String password) {
  _ssid = ssid;
  _password = password;

  while (_serial.available() > 0) {
    _serial.read();
  }

  sendCommand("AT+RST", "111", "222");
  sendCommand("AT", "ESP8266 is found!", "NO ESP8266 is found");
  sendCommand("AT+GMR", "333", "444");

  sendCommand("AT+CWMODE?", "111", "222", true);

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


        sendCommand("AT+CIPSERVER=0,8080", "", "");
        //      _serial.println();
        //      delay(500);
        //      if(_serial.find("OK")) {
        //        Serial.println("Yay");
        //      } else {
        //        Serial.println("Nay");
        //      }

        sendCommand("AT+CWMODE=1", "", "");
        //      _serial.println("AT+CWMODE=1");
        //      delay(500);
        //      if(_serial.find("OK")) {
        //        Serial.println("Yay");
        //      } else {
        //        Serial.println("Nay");
        //      }

        String jCmd = "AT+CWJAP=\"";
        jCmd += ssid;
        jCmd += "\",\"";
        jCmd += pass;
        jCmd += "\"";

        //sendCommand(jCmd, "", "");
        _serverIsRunning = false;
        Serial.println(jCmd);

        _serial.println(jCmd);
        delay(5000);
        if (_serial.find("OK")) {
          Serial.println("Connected");
          return;
        } else {
          Serial.println("Connection failed");
        }
      }


    }
  }
}

void ESP8266EasyConfig::end() {
  sendCommand("AT+CWQAP", "", "");
}

void ESP8266EasyConfig::reset() {
  end();
  begin(_ssid, _password);
}

boolean ESP8266EasyConfig::setupAP() {
  _serial.println("AT+CIPMUX=1");
  if (_serial.find("OK"))
  {
    Serial.println(F("Multiconnection mode enabled"));
  }
  else
  {
    Serial.println(F("Multiconnection mode failed"));
  }

  _serial.println("AT+CWMODE=2");
  delay(1000);
  if (_serial.find("OK"))
  {
    Serial.println(F("Acce_serial Point mode enabled"));
  }
  else
  {
    Serial.println(F("Acce_serial Point mode failed to initiate"));

  }

  _serial.println(F("AT+CIPSERVER=1,8080"));
  delay(1000);
  if (_serial.find("OK"))
  {
    Serial.println(F("Server is running"));
  }
  else
  {
    Serial.println(F("Server initiated failed"));
  }

  _serial.println(F("AT+CWSAP=\"MORAS\",\"pa_serialword\",11,0")); //Setup the AP with _serialID MORAS, on channel 11, with no pa_serialword
  delay(1000);
  if (_serial.find("OK"))
  {
    Serial.println(F("OK, accespoint is operational"));
  }
  else
  {
    Serial.println(F("FAILED, accespoint failed"));
  }
  return true;
}

String ESP8266EasyConfig::sendCommand(String command, String success, String failure, boolean transaction) {
  _serial.println(command);
  delay(1000);
  String ret = readCom();
  if (transaction) {
    Serial.println("Transaction:");
    Serial.println(ret);
  }
  return ret;
}

String ESP8266EasyConfig::readCom() {
  String ret = "";
  _rbIndex = 0;

  while (_serial.available() > 0) {
    if (_rbIndex > (BUFFER_SIZE - 2)) {
      Serial.println("Read buffer full :<");
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

  String ret = _serial.readStringUntil('\n');
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
    Serial.println("Data malformed?");
    Serial.println(ret);
    ret = "";
  }

  return ret;
}
