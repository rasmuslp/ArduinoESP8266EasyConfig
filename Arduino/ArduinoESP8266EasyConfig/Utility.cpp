#include "Utility.h"

String Utility::findValue(String input, String key) {
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
      input = v;
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
