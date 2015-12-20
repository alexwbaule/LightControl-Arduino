#include "Config.h"


WiFiManager::WiFiManager(bool debug) : server(80) {
  _debug = debug;
}

void WiFiManager::begin(char const *apName) {
  DEBUG_PRINT("");
  _apName = apName;
  start = millis();

  DEBUG_PRINT("Configuring access point... ");
  DEBUG_PRINT(_apName);
  //optional soft ip config
  if (_ip) {
    DEBUG_PRINT("Custom IP/GW/Subnet");
    WiFi.softAPConfig(_ip, _gw, _sn);
  }

  WiFi.softAP(_apName);//TODO: add password option
  delay(500); // Without delay I've seen the IP address blank
  DEBUG_PRINT("AP IP address: ");
  DEBUG_PRINT(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */  
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server.on("/", std::bind(&WiFiManager::handleNotFound, this));
  server.on("/scan", std::bind(&WiFiManager::handleWifiJSON, this));
  server.on("/state", std::bind(&WiFiManager::handleState, this));
  server.on("/wifichange", std::bind(&WiFiManager::handleWifiSave, this));
  server.on("/generate_204", std::bind(&WiFiManager::handle204, this));  //Android/Chrome OS captive portal check.
  server.onNotFound (std::bind(&WiFiManager::handleNotFound, this));
  server.begin(); // Web server start
  DEBUG_PRINT("HTTP server started");
  setmDNS();
}

/** Wifi config page handler */
void WiFiManager::handleState() {
  String s = "{'config' : 'false',";
  s += "'signal' : '" + (String)WiFi.RSSI() + "'";
  s += "}";

  sendHeader(true, 200, s.c_str());
  server.client().stop();
  DEBUG_PRINT("Sent StatePage");  
}

void WiFiManager::setmDNS(){
  devDNS = "lgt" + String(ESP.getChipId());
  DEBUG_PRINT(devDNS);  
  if (mdns.begin(devDNS.c_str())) {
    DEBUG_PRINT("DNS Started OK");  
    _dns = true;
    mdns.addService("light", "tcp", 80);
    mdns.update();
  }
}

int WiFiManager::autoConnect() {
  String ssid = "lgt" + String(ESP.getChipId());
  return autoConnect(ssid.c_str());
}

int WiFiManager::autoConnect(char const *apName) {
  DEBUG_PRINT("");
  DEBUG_PRINT("AutoConnect");
  
  String ssid = WiFi.SSID();
  String pass = WiFi.psk();

  DEBUG_PRINT("SSID and Passwd");
  DEBUG_PRINT(ssid);
  DEBUG_PRINT(pass);

  WiFi.mode(WIFI_STA);
  connectWifi(ssid, pass);
  int s = WiFi.status();
  if (s == WL_CONNECTED) {
    DEBUG_PRINT("IP Address:");
    DEBUG_PRINT(WiFi.localIP());
    //connected
    return 1;
  }

  WiFi.mode(WIFI_AP);


  connect = false;
  begin(apName);

  while(1) {
    //DNS
    dnsServer.processNextRequest();
    //HTTP
    server.handleClient();
    
    if(connect) {
      DEBUG_PRINT("Connecting to new AP");
      connect = false;
      connectWifi(_ssid, _pass);
      int s = WiFi.status();
      if (s == WL_CONNECTED) {
        //connected
        return 2;
      }
    }
    //mdns.update();
    yield();    
  }
  return 0;
}

void WiFiManager::connectWifi(String ssid, String pass) {
  DEBUG_PRINT("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin(ssid.c_str(), pass.c_str());
  int connRes = WiFi.waitForConnectResult();
  DEBUG_PRINT ("Connection result: ");
  DEBUG_PRINT ( connRes );
}

String WiFiManager::urldecode(const char *src){
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

void WiFiManager::setTimeout(unsigned long seconds) {
  timeout = seconds * 1000;
}

void WiFiManager::setDebugOutput(boolean debug) {
  _debug = debug;
}

void WiFiManager::setAPConfig(IPAddress ip, IPAddress gw, IPAddress sn) {
  _ip = ip;
  _gw = gw;
  _sn = sn;
}

void WiFiManager::handleWifiJSON() {
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

/** Handle the WLAN save form and redirect to WLAN config page again */
void WiFiManager::handleWifiSave() {
  DEBUG_PRINT("WiFi save");
  String s = "{'saved' : 'true'}";
  sendHeader(true, 200, s.c_str());
  server.client().stop();
  
  DEBUG_PRINT("Sent wifi save page"); 
  _ssid = urldecode(server.arg("s").c_str());
  _pass = urldecode(server.arg("p").c_str());
  _name = urldecode(server.arg("n").c_str());

  DEBUG_PRINT(_name);
  bool ret = ehand.setDevName(_name);
  DEBUG_PRINT(ret);
  
  //saveCredentials();
  connect = true; //signal ready to connect/reset
}

void WiFiManager::handle204() {
  DEBUG_PRINT("204 No Response");  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 204, "text/plain", "");
}


void WiFiManager::handleNotFound() {
  if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
    return;
  }
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
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 404, "text/plain", message );
}


/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean WiFiManager::captivePortal() {
  if (!isIp(server.hostHeader()) ) {
    DEBUG_PRINT("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}
void WiFiManager::sendHeader(bool json, int cod, const char *content){
  String type;

  type = json ? "application/json" : "text/html" ;

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(cod, type.c_str(), content);
}


template <typename Generic>
void WiFiManager::DEBUG_PRINT(Generic text) {
  if(_debug) {
    Serial.print("*WM: ");
    Serial.println(text);    
  }
}

/** Is this an IP? */
boolean WiFiManager::isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String WiFiManager::toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}


