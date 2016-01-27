#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Config.h"
#include "Control.h"

Control ctrl;
//boolean debug = false;
boolean debug = true;
int times = 7;


void checkButtonTime(){
  int reading = digitalRead(ctrl.pin_button);

  if (reading != ctrl.lastbutton_state) {
    ctrl.lastDebounceTime = millis();
  }
  if ((millis() - ctrl.lastDebounceTime) > ctrl.debounceDelay) {
    if (reading != ctrl.button_state) {
      ctrl.button_state = reading;
      if (ctrl.button_state == LOW) {
        ctrl.light_state = !ctrl.light_state;
      }
    }
  }
  digitalWrite(ctrl.pin_light,ctrl.light_state);
  ctrl.lastbutton_state = reading;
}

void setup() {
  pinMode(ctrl.pin_light, OUTPUT);
  pinMode(ctrl.pin_button, INPUT);


  digitalWrite(ctrl.pin_light, ctrl.light_state);

  if(debug){
    Serial.begin(115200);
    Serial.print("wait is ");
    Serial.println(times);
  }

  WiFiManager wifi(debug);
  int r = wifi.autoConnect();

  if(r == 2){
    while(1){
      delay(1000);
      yield();
      times++;
      if(times >= 7)
        break;     
    }
    if(debug)
        Serial.println("Rebooting....");
    ESP.restart();
  }

  ctrl.begin(debug);
  
  if(debug)
    Serial.println("Wifi OK, pode usar agora....");
}

void loop() {
    checkButtonTime();
    ctrl.handleClient();
}
