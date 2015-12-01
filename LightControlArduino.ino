#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Config.h"
#include "Control.h"

Control ctrl;
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
  ctrl.light_state = !ctrl.light_state;
  digitalWrite(ctrl.pin_light,ctrl.light_state);
}

void setup() {
  if(debug)
    Serial.begin(115200);
  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, ctrl.light_state);
  //prepara GPIO0
  pinMode(0, INPUT_PULLUP);


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
    checkButton();
    ctrl.handleClient();
}
