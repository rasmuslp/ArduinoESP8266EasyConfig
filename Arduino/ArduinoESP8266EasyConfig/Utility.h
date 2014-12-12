#ifndef Utility_h
#define Utility_h

#include "Arduino.h"

class Utility {
  public:
    static String findValue(String input, const String key);
    static int8_t findNoOccurrences(const String input, const String searchingFor);
};

#endif
