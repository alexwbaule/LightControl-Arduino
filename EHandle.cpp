#include "EHandle.h"

EHandle::EHandle(){
}
void EHandle::getDevName(){
  EEPROM.begin(512);
  EEPROM.get(0, lastname);
  EEPROM.end();
}

bool EHandle::setDevName(String thename){
  if (strcmp(lastname, thename.c_str()) != 0){
      strcpy(lastname, thename.c_str());
      EEPROM.begin(512);
      EEPROM.put(0, lastname);
      EEPROM.end();
      return true;
  }
  return false;
}
