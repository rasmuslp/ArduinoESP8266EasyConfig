#ifndef common_h
#define common_h

#define DEBUG

#ifdef DEBUG
#define DBG(message) Serial.print(message)
#define DBGLN(message) Serial.println(message)
#else
#define DBG(message)
#define DBGLN(message)
#endif

#endif
