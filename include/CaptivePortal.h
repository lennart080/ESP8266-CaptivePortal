// Licensed under CC BY-NC 4.0
// https://creativecommons.org/licenses/by-nc/4.0/
// © 2025 Lennart Gutjahr

#pragma once
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <DNSServer.h>

enum class CaptivePortalError {
    None,
    AlreadyRunning,
    FSInitFailed,
    APStartFailed,
    DNSServerStartFailed,
    NotRunning,
    Unknown,
    FileNotFound,
    SoftAPDisconnectFailed,
    InvalidSSID,
    InvalidPassword,
    NotInitialized,
    AlreadyInitialized
};

class CaptivePortal {
public:
    bool initialize(const char* ssid, const char* defaultFile = "index.html");
    bool initialize(const char* ssid, const char* password, const char* defaultFile = "index.html");
    bool stopAP();
    bool startAP();
    void processDNS();
    AsyncWebServer& getServer();
    CaptivePortalError getLastError() const;

private:
    void registerRoutes();

    AsyncWebServer server = AsyncWebServer(80);
    DNSServer dnsServer;
    bool apRunning = false;
    bool initialized = false;
    CaptivePortalError lastError = CaptivePortalError::None;
};
