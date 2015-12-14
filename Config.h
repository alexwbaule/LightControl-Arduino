#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include "EHandle.h"


class WiFiManager
{
public:
    WiFiManager(bool debug);
    void    begin(char const *apName);
    
    int autoConnect();
    int autoConnect(char const *apName);
    
    boolean hasConnected();
    
    String  beginConfigMode(void);
    void    startWebConfig();
    void setmDNS();
    
    //for conveniennce
    String  urldecode(const char*);

    //sets timeout before webserver loop ends and exits even if there has been no setup. 
    //usefully for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds
    void    setTimeout(unsigned long seconds);
    void    setDebugOutput(boolean debug);

    //sets a custom ip /gateway /subnet configuration
    void    setAPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    
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
    const String HTTP_FORM = "<form method='get' action='wifisave'><input id='n' name='n' length=63 placeholder='Nome'><input id='s' name='s' length=32 placeholder='SSID'><input id='p' name='p' length=64 placeholder='password'><br/><button type='submit'>save</button></form>";
    const String HTTP_SAVED = "<div>Credentials Saved<br />Node will reboot in 5 seconds.</div>";
    const String HTTP_END = "</body></html>";
    
    String devDNS;
    int _eepromStart;
    const char* _apName = "no-net";
    String _ssid = "";
    String _pass = "";
    String _name = "";
    unsigned long timeout = 0;
    unsigned long start = 0;
    IPAddress _ip;
    IPAddress _gw;
    IPAddress _sn;
    
    bool keepLooping = true;
    int status = WL_IDLE_STATUS;
    void connectWifi(String ssid, String pass);

    void handleRoot();
    void handleWifi(bool scan);
    void handleWifiJSON();
    void handleWifiSave();
    void handleNotFound();
    void handle204();
    boolean captivePortal();
    boolean _dns = false;

    // DNS server
    const byte DNS_PORT = 53;
    
    boolean isIp(String str);
    String toStringIp(IPAddress ip);

    void sendHeader(bool json, int cod, const char *content);
    void handleState();


    boolean connect;
    boolean _debug = false;

    template <typename Generic>
    void DEBUG_PRINT(Generic text);
};



#endif
