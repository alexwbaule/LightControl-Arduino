#include "Control.h"


Control::Control() : server(80) {
}

void Control::begin(bool debug){
  _debug = debug;
  DEBUG_PRINT("Configuring access point... ");
  ehand.getDevName();
  DEBUG_PRINT(ehand.lastname);
  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server.on("/", std::bind(&Control::handleNotFound, this));
  server.on("/light/off", std::bind(&Control::handleCmd, this, false));
  server.on("/light/on", std::bind(&Control::handleCmd, this, true));
  server.on("/state", std::bind(&Control::handleState, this));
  server.on("/scan", std::bind(&Control::handleWifi, this));
  server.on("/wifichange", std::bind(&Control::handleWifiSave, this));
  server.onNotFound (std::bind(&Control::handleNotFound, this));
  server.begin(); // Web server start
  DEBUG_PRINT("HTTP server started");
  setmDNS();
}

void Control::setmDNS(){
  devDNS = "lgt" + String(ESP.getChipId());
  DEBUG_PRINT(devDNS);  
  if (mdns.begin(devDNS.c_str())) {
    DEBUG_PRINT("DNS Started OK");  
    _dns = true;
    mdns.addService("light", "tcp", 80);
    mdns.update();
  }
}

void Control::handleClient(){
    server.handleClient();
}

/** Wifi config page handler */
void Control::handleState() {
  String s = "{'config' : 'true', 'name' : '" + (String)ehand.lastname + "',";
  s += (light_state)?"'state': 'on',":"'state': 'off',";
  s += "'signal' : '" + (String)WiFi.RSSI() + "'";
  s += "}";
  sendHeader(true, 200, s.c_str());
  server.client().stop();
  DEBUG_PRINT("Sent StatePage");  
}


/** Wifi config page handler */
void Control::handleCmd(bool state) {
  light_state = state;
  digitalWrite(pin_light,light_state);

  String s = "{'name' : '" + (String)ehand.lastname + "',";
  s += (light_state)?"'state': 'on',":"'state': 'off',";
  s += "'signal' : '" + (String)WiFi.RSSI() + "'";
  s += "}";
  sendHeader(true, 200, s.c_str());
  if(_dns){
      mdns.update();
  }
  
  server.client().stop();
  
  DEBUG_PRINT("Sent Cmd page");  
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void Control::handleWifiSave() {
  DEBUG_PRINT("WiFi save");
  String s = "{'saved' : 'true'}";
  sendHeader(true, 200, s.c_str());
  server.client().stop();
  
  DEBUG_PRINT("Sent wifi save page"); 
  String _ssid = urldecode(server.arg("s").c_str());
  String _pass = urldecode(server.arg("p").c_str());
  String _name = urldecode(server.arg("n").c_str());
  bool ret = ehand.setDevName(_name);
  String ssid = WiFi.SSID();
  String pass = WiFi.psk();

  if(!ssid.equals(_ssid) && !pass.equals(_pass)){
    DEBUG_PRINT("Dados Alterados, reiniciando...");  
    delay(5000);
    WiFi.disconnect();
    WiFi.begin(_ssid.c_str(), _pass.c_str());
    ESP.restart();
  }
}

void Control::handleWifi() {
  String json;
  json = "{\"apssid\": [";
  
  int n = WiFi.scanNetworks();
  DEBUG_PRINT("Scan done");
  if (n == 0) {
    DEBUG_PRINT("No networks found");
  } else {
    for (int i = 0; i < n; ++i){
      json += "{\"ssid\" : \"" + WiFi.SSID(i) + "\",\"mac\": \"" + WiFi.BSSIDstr(i) + "\", \"signal\" : \"" + WiFi.RSSI(i) + "\"}";
      if((i + 1) < n)
        json += ",";
      yield();
    }
  }
  json += "]}";
  sendHeader(true, 200, json.c_str());
  server.client().stop();
  DEBUG_PRINT("Sent config page");  
}

void Control::handle204() {
  DEBUG_PRINT("204 No Response");  
  sendHeader(false, 204, "");
}


void Control::handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  sendHeader(false, 404, message.c_str());
}

template <typename Generic>
void Control::DEBUG_PRINT(Generic text) {
  if(_debug) {
    Serial.print("*Ctrl: ");
    Serial.println(text);    
  }
}

void Control::sendHeader(bool json, int cod, const char *content){
  String type;

  type = json ? "application/json" : "text/html" ;

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(cod, type.c_str(), content);
}

String Control::urldecode(const char *src){
  String decoded = "";
  char a, b;
  while (*src) {
    if ((*src == '%') &&
        ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';

      decoded += char(16 * a + b);
      src += 3;
    } else if (*src == '+') {
      decoded += ' ';
      *src++;
    } else {
      decoded += *src;
      *src++;
    }
  }
  return decoded;
}

void Control::setDebugOutput(boolean debug) {
  _debug = debug;
}

/** Is this an IP? */
boolean Control::isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String Control::toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}


