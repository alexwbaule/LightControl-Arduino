#ifndef EHandle_h
#define EHandle_h

#include <EEPROM.h>
#include <ESP8266WiFi.h>

#define MAX_NAME 64

class EHandle
{
public:
  EHandle();
  void getDevName();
  bool setDevName(String thename);
  char lastname[MAX_NAME];
};
#endif
