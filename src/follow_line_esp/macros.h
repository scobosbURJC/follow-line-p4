#include "credentials.h"

#define DEBUG

#ifdef DEBUG
  #define TRACE(msg) Serial.print(msg)
  #define TRACE_LN(msg) Serial.println(msg)
#else
  #define TRACE(msg)
  #define TRACE_LN(msg)
#endif