#ifndef Control_h
#define Control_h

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include "EHandle.h"

class Control
{
public:
    Control();
    void    begin(bool debug);
    
    //for conveniennce
    String  urldecode(const char*);
    void handleClient();
    void setmDNS();
    void setDebugOutput(boolean debug);

    int button_state = 0;
    int light_state = LOW;
    int lastbutton_state = LOW;
    long lastDebounceTime = 0;  // the last time the output pin was toggled
    long debounceDelay = 50;    // the debounce time; increase if the output flickers

    //DEBUG
    int pin_light = 2; //GPIO2
    int pin_button = 0; //GPIO0
    //END DEBUG

    // PRODUCTION
    //int pin_light = 1; //GPIO1 - TX;
    //int pin_button = 3; //GPIO3 - RX;
    // END PRODUCTION


private:
    DNSServer dnsServer;
    ESP8266WebServer server;
    MDNSResponder mdns;


    EHandle ehand;
    const int WM_DONE = 0;
    const int WM_WAIT = 10;

    const String HTTP_404 = "HTTP/1.1 404 Not Found\r\n\r\n";
    const String HTTP_200 = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    const String HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";
    const String HTTP_STYLE = "<style>div,input {margin-bottom: 5px;} body{width:200px;display:block;margin-left:auto;margin-right:auto;} button{padding:0.75rem 1rem;border:0;border-radius:0.317rem;background-color:#1fa3ec;color:#fff;line-height:1.5;cursor:pointer;}</style>";
    const String HTTP_SCRIPT = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
    const String HTTP_HEAD_END = "</head><body>";
    const String HTTP_ITEM = "<div><a href='#' onclick='c(this)'>{v}</a>&nbsp;{a}</div>";
    const String HTTP_FORM = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID'><input id='p' name='p' length=64 placeholder='password'><br/><button type='submit'>save</button></form>";
    const String HTTP_SAVED = "<div>Credentials Saved<br />Node will reboot in 5 seconds.</div>";
    const String HTTP_END = "</body></html>";

    String devDNS;

    void handleCmd(bool state);
    void handleState();

    void handleWifiSave();
    void handleWifi();
    void handleNotFound();
    void handle204();

    void sendHeader(bool json, int cod, const char *content);

    boolean isIp(String str);
    String toStringIp(IPAddress ip);

    boolean connect;
    boolean _dns = false;
    boolean _debug = false;

    template <typename Generic>
    void DEBUG_PRINT(Generic text);
};
#endif
