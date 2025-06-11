// Licensed under CC BY-NC 4.0
// https://creativecommons.org/licenses/by-nc/4.0/
// © 2025 Lennart Gutjahr

#pragma once
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <DNSServer.h>

class CaptivePortal {
public:
    bool initialize(const char* ssid = "MyAP", const char* password = nullptr, const char* defaultFile = "index.html");
    bool stopAP();
    bool startAP();
    void processDNS();
    AsyncWebServer& getServer();

private:
    void registerRoutes(const char* defaultFile);

    AsyncWebServer server = AsyncWebServer(80);
    DNSServer dnsServer;
    bool apRunning = false;
};
