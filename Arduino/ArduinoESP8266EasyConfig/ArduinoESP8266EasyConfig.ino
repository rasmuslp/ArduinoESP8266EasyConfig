#include <SoftwareSerial.h>

#define SOFT_SERIAL_RX 7
#define SOFT_SERIAL_TX 8

#define LED_RED 3
#define LED_GREEN 4
#define LED_BLUE 5

#define RESET_BTN 6

SoftwareSerial dSerial(SOFT_SERIAL_RX, SOFT_SERIAL_TX); // RX, TX

String ssid;
String pass;
boolean serverIsRunning = false;

#define BUFF 512
char readBuf[BUFF];
byte rbIndex = 0;

void setup() {
  dSerial.begin(9600);
  dSerial.println("ESP8266 AP setup");

  pinMode(RESET_BTN, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  Serial.begin(9600);
  Serial.setTimeout(5000);
  while (!Serial);
  while (Serial.available() > 0) {
    Serial.read();
  }

  sendCommand("AT+RST", "111", "222");

  sendCommand("AT", "ESP8266 is found!", "NO ESP8266 is found");

  sendCommand("AT+GMR", "333", "444");
}

String receiveData() {
  if (!Serial.find("+IPD,")) {
    return "";
  }

  String ret = Serial.readStringUntil('\n');
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
    dSerial.print("Conn ID: ");
    dSerial.println(ret.substring(0, connectionIdDelimiter));
    ret = ret.substring(startDelimiter + 1);
  } else {
    dSerial.println("Data malformed?");
    dSerial.println(ret);
    ret = "";
  }

  return ret;
}

String readCom() {
  String ret = "";
  rbIndex = 0;

  while (Serial.available() > 0) {
    if (rbIndex > (BUFF - 2)) {
      dSerial.println("Read buffer full :<");
      //return false;
    }
    char c = Serial.read();
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
  Serial.println(command);
  delay(1000);
  String ret = readCom();
  dSerial.println("Transaction:");
  dSerial.println(ret);
  return true;
}

String findValue(String input, String key) {
  dSerial.println("Got: " + input);

  String ret = "Key not found: " + key;
  int eqIndex = input.indexOf('=');
  while (eqIndex > -1) {
    String k = input.substring(0, eqIndex);
    dSerial.println("Found key: " + k);
    String v;
    int ampIndex = input.indexOf('&');
    if (ampIndex > -1) {
      v = input.substring(eqIndex + 1, ampIndex);
      input = input.substring(ampIndex+1);
    } else {
      v = input.substring(eqIndex + 1);
    }
    dSerial.println("Found value: " + v);
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
  //  dSerial.println(digitalRead(RESET_BTN));

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
      dSerial.print("Invalid request: ");
      dSerial.println(readValue);
      return;
    }
    int reqEnd = readValue.lastIndexOf(" HTTP/");
    if (reqEnd == -1) {
      // No request end, leaving...
      dSerial.print("Invalid request: ");
      dSerial.println(readValue);
      return;
    }

    readValue = readValue.substring(reqStart + 5, reqEnd);

    //TODO: Remove
    dSerial.println(readValue);

    // Read command
    int commandDelimiter = readValue.indexOf('/');
    if (commandDelimiter == -1) {
      // No command delimiter, leaving...
      dSerial.print("Invalid command: ");
      dSerial.println(readValue);
      return;
    }
    String command = readValue.substring(0, commandDelimiter);
    dSerial.println("Command:");
    dSerial.println(command);

    //TODO
    readValue = readValue.substring(commandDelimiter+2);

    // Parse commands
    if (command.equals("join")) {
      dSerial.println("Got join command");
      String ssid = findValue(readValue, "ssid");
      String pass = findValue(readValue, "pass");
      dSerial.print("SSID: ");
      dSerial.println(ssid);
      dSerial.print("Password: ");
      dSerial.println(pass);
      
      
      sendCommand("AT+CIPSERVER=0,8080", "", "");
//      Serial.println();
//      delay(500);
//      if(Serial.find("OK")) {
//        dSerial.println("Yay");
//      } else {
//        dSerial.println("Nay");
//      }
      
            sendCommand("AT+CWMODE=1", "", "");
//      Serial.println("AT+CWMODE=1");
//      delay(500);
//      if(Serial.find("OK")) {
//        dSerial.println("Yay");
//      } else {
//        dSerial.println("Nay");
//      }
      
      String jCmd = "AT+CWJAP=\"";
      jCmd += ssid;
      jCmd += "\",\"";
      jCmd += pass;
      jCmd += "\"";
      
      //sendCommand(jCmd, "", "");
      
      dSerial.println(jCmd);
      
      Serial.println(jCmd);
      delay(5000);
      if(Serial.find("OK")) {
        digitalWrite(LED_GREEN, HIGH);
      } else {
        digitalWrite(LED_RED, HIGH);
      }
    }
  }

  //  String cmd = "AT+CIPSTART=\"TCP\",\"";
  //  cmd += DST_IP;
  //  cmd += "\",80";
  //  Serial.println(cmd);
  //  dbgSerial.println(cmd);
  //  if(Serial.find("Error")) return;
  //  cmd = "GET / HTTP/1.0\r\n\r\n";
  //  Serial.print("AT+CIPSEND=");
  //  Serial.println(cmd.length());
  //  if(Serial.find(">"))
  //  {
  //    dbgSerial.print(">");
  //  }
  //  else
  //  {
  //    Serial.println("AT+CIPCLOSE");
  //    dbgSerial.println("connect timeout");
  //    delay(1000);
  //    return;
  //  }
  //  Serial.print(cmd);
  //  delay(2000);
  //  //Serial.find("+IPD");
  //  while (Serial.available())
  //  {
  //    char c = Serial.read();
  //    dbgSerial.write(c);
  //    if(c=='\r') dbgSerial.print('\n');
  //  }
  //  dbgSerial.println("====");
  //  delay(1000);
}

//
//boolean connectWiFi()
//{
//  Serial.println("AT+CWMODE=1");
//  String cmd="AT+CWJAP=\"";
//  cmd+=SSID;
//  cmd+="\",\"";
//  cmd+=PASS;
//  cmd+="\"";
//  dbgSerial.println(cmd);
//  Serial.println(cmd);
//  delay(2000);
//  if(Serial.find("OK"))
//  {
//    dSerial.println("OK, Connected to WiFi.");
//    return true;
//  }
//  else
//  {
//    dbgSerial.println("Can not connect to the WiFi.");
//    return false;
//  }
//}



boolean setupAsAP()
{
  Serial.println("AT+CIPMUX=1");
  if (Serial.find("OK"))
  {
    dSerial.println("Multiconnection mode enabled");
  }
  else
  {
    dSerial.println("Multiconnection mode failed");
  }

  Serial.println("AT+CWMODE=2");
  delay(1000);
  if (Serial.find("OK"))
  {
    dSerial.println("Access Point mode enabled");
  }
  else
  {
    dSerial.println("Access Point mode failed to initiate");

  }

  Serial.println("AT+CIPSERVER=1,8080");
  delay(1000);
  if (Serial.find("OK"))
  {
    dSerial.println("Server is running");
  }
  else
  {
    dSerial.println("Server initiated failed");
  }

  Serial.println("AT+CWSAP=\"MORAS\",\"password\",11,0"); //Setup the AP with SSID MORAS, on channel 11, with no password
  delay(1000);
  if (Serial.find("OK"))
  {
    dSerial.println("OK, accespoint is operational");
  }
  else
  {
    dSerial.println("FAILED, accespoint failed");
  }
  return true;
}



