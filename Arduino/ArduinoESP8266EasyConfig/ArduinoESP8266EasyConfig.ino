#include <SoftwareSerial.h>

#define SOFT_SERIAL_RX 7
#define SOFT_SERIAL_TX 8

#define LED_RED 3
#define LED_GREEN 4
#define LED_BLUE 5

#define RESET_BTN 6

SoftwareSerial ss(SOFT_SERIAL_RX, SOFT_SERIAL_TX); // RX, TX

String ssid;
String pass;
boolean serverIsRunning = false;

#define BUFF 512
char readBuf[BUFF];
byte rbIndex = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("ESP8266 AP setup");

  pinMode(RESET_BTN, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  ss.begin(9600);
  ss.setTimeout(5000);
  while (ss.available() > 0) {
    ss.read();
  }

  sendCommand("AT+RST", "111", "222");

  sendCommand("AT", "ESP8266 is found!", "NO ESP8266 is found");

  sendCommand("AT+GMR", "333", "444");
}

String receiveData() {
  if (!ss.find("+IPD,")) {
    return "";
  }

  String ret = ss.readStringUntil('\n');
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

String readCom() {
  String ret = "";
  rbIndex = 0;

  while (ss.available() > 0) {
    if (rbIndex > (BUFF - 2)) {
      Serial.println("Read buffer full :<");
      //return false;
    }
    char c = ss.read();
    readBuf[rbIndex++] = c;
    readBuf[rbIndex] = '\0';
  }

  if (rbIndex > 0) {
    ret = readBuf;
    readBuf[0] = '\0';
  }

  return ret;
}

boolean sendCommand(String command, String success, String failure) {
  ss.println(command);
  delay(1000);
  String ret = readCom();
  Serial.println("Transaction:");
  Serial.println(ret);
  return true;
}

String findValue(String input, String key) {
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
      input = input.substring(ampIndex+1);
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

void loop() {
  //  Serial.println(digitalRead(RESET_BTN));

  if (!serverIsRunning) {
    serverIsRunning = setupAsAP();
  } else {
    String readValue = receiveData();
    if (readValue.equals("")) {
      return;
    }

    // Verify request start and end
    int reqStart = readValue.indexOf("GET /");
    if (reqStart == -1) {
      // No request start, leaving...
      Serial.print("Invalid request: ");
      Serial.println(readValue);
      return;
    }
    int reqEnd = readValue.lastIndexOf(" HTTP/");
    if (reqEnd == -1) {
      // No request end, leaving...
      Serial.print("Invalid request: ");
      Serial.println(readValue);
      return;
    }

    readValue = readValue.substring(reqStart + 5, reqEnd);

    //TODO: Remove
    Serial.println(readValue);

    // Read command
    int commandDelimiter = readValue.indexOf('/');
    if (commandDelimiter == -1) {
      // No command delimiter, leaving...
      Serial.print("Invalid command: ");
      Serial.println(readValue);
      return;
    }
    String command = readValue.substring(0, commandDelimiter);
    Serial.println("Command:");
    Serial.println(command);

    //TODO
    readValue = readValue.substring(commandDelimiter+2);

    // Parse commands
    if (command.equals("join")) {
      Serial.println("Got join command");
      String ssid = findValue(readValue, "ssid");
      String pass = findValue(readValue, "pass");
      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("Password: ");
      Serial.println(pass);
      
      
      sendCommand("AT+CIPSERVER=0,8080", "", "");
//      ss.println();
//      delay(500);
//      if(ss.find("OK")) {
//        Serial.println("Yay");
//      } else {
//        Serial.println("Nay");
//      }
      
            sendCommand("AT+CWMODE=1", "", "");
//      ss.println("AT+CWMODE=1");
//      delay(500);
//      if(ss.find("OK")) {
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
      
      Serial.println(jCmd);
      
      ss.println(jCmd);
      delay(5000);
      if(ss.find("OK")) {
        digitalWrite(LED_GREEN, HIGH);
      } else {
        digitalWrite(LED_RED, HIGH);
      }
    }
  }

  //  String cmd = "AT+CIPSTART=\"TCP\",\"";
  //  cmd += DST_IP;
  //  cmd += "\",80";
  //  ss.println(cmd);
  //  dbgss.println(cmd);
  //  if(ss.find("Error")) return;
  //  cmd = "GET / HTTP/1.0\r\n\r\n";
  //  ss.print("AT+CIPSEND=");
  //  ss.println(cmd.length());
  //  if(ss.find(">"))
  //  {
  //    dbgss.print(">");
  //  }
  //  else
  //  {
  //    ss.println("AT+CIPCLOSE");
  //    dbgss.println("connect timeout");
  //    delay(1000);
  //    return;
  //  }
  //  ss.print(cmd);
  //  delay(2000);
  //  //ss.find("+IPD");
  //  while (ss.available())
  //  {
  //    char c = ss.read();
  //    dbgss.write(c);
  //    if(c=='\r') dbgss.print('\n');
  //  }
  //  dbgss.println("====");
  //  delay(1000);
}

//
//boolean connectWiFi()
//{
//  ss.println("AT+CWMODE=1");
//  String cmd="AT+CWJAP=\"";
//  cmd+=SSID;
//  cmd+="\",\"";
//  cmd+=PASS;
//  cmd+="\"";
//  dbgss.println(cmd);
//  ss.println(cmd);
//  delay(2000);
//  if(ss.find("OK"))
//  {
//    Serial.println("OK, Connected to WiFi.");
//    return true;
//  }
//  else
//  {
//    dbgss.println("Can not connect to the WiFi.");
//    return false;
//  }
//}



boolean setupAsAP()
{
  ss.println("AT+CIPMUX=1");
  if (ss.find("OK"))
  {
    Serial.println("Multiconnection mode enabled");
  }
  else
  {
    Serial.println("Multiconnection mode failed");
  }

  ss.println("AT+CWMODE=2");
  delay(1000);
  if (ss.find("OK"))
  {
    Serial.println("Access Point mode enabled");
  }
  else
  {
    Serial.println("Access Point mode failed to initiate");

  }

  ss.println("AT+CIPSERVER=1,8080");
  delay(1000);
  if (ss.find("OK"))
  {
    Serial.println("Server is running");
  }
  else
  {
    Serial.println("Server initiated failed");
  }

  ss.println("AT+CWSAP=\"MORAS\",\"password\",11,0"); //Setup the AP with SSID MORAS, on channel 11, with no password
  delay(1000);
  if (ss.find("OK"))
  {
    Serial.println("OK, accespoint is operational");
  }
  else
  {
    Serial.println("FAILED, accespoint failed");
  }
  return true;
}



