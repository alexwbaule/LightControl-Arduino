#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Config.h"
#include "Control.h"

Control ctrl;
//boolean debug = false;
boolean debug = true;


void checkButton(){
  ctrl.button_state = digitalRead(ctrl.pin_button);
  
  if(ctrl.lastbutton_state == ctrl.button_state){
    return;
  }
  ctrl.lastbutton_state = ctrl.button_state;
  
  if (ctrl.button_state == LOW){
    return;
  }
  delay(100);
  ctrl.light_state = !ctrl.light_state;
  digitalWrite(ctrl.pin_light,ctrl.light_state);
}

void checkButtonTime(){
  int reading = digitalRead(ctrl.pin_button);

  if (reading != ctrl.lastbutton_state) {
    ctrl.lastDebounceTime = millis();
  }
  if ((millis() - ctrl.lastDebounceTime) > ctrl.debounceDelay) {
    if (reading != ctrl.button_state) {
      ctrl.button_state = reading;
      if (ctrl.button_state == HIGH) {
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

  if(debug)
    Serial.begin(115200);


  WiFiManager wifi(debug);
  int r = wifi.autoConnect();

  if(r == 2){
    delay(5000);
    if(debug)
        Serial.println("Rebooting....");
    ESP.restart();
  }

  ctrl.begin(debug);
  if(debug)
    Serial.println("Wifi OK, pode usar agora....");
}

void loop() {
    //checkButton();
    checkButtonTime();
    ctrl.handleClient();
}
