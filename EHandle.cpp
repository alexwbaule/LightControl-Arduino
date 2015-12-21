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
      clear();
      EEPROM.begin(512);
      EEPROM.put(0, lastname);
      EEPROM.end();
      return true;
  }
  return false;
}

void EHandle::clear(){
  EEPROM.begin(512);
  for (int i = 0; i < 512; i++)
    EEPROM.write(i, 0);
  EEPROM.end();
}

