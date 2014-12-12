#include "Utility.h"

String Utility::findValue(String input, const String key) {
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

int8_t Utility::findNoOccurrences(const String input, const String searchingFor) {
  int last = input.lastIndexOf(searchingFor);
  if (last == -1) {
    return 0;
  }
  
  int8_t occurrences = 1;
  int first = input.indexOf(searchingFor);
  while (first < last) {
    occurrences++;
    first = input.indexOf(searchingFor, first + 1);
  }
  
  return occurrences;
}
