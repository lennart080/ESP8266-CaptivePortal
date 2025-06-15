// Licensed under Apache License, Version 2.0
// SPDX-License-Identifier: Apache-2.0
// http://www.apache.org/licenses/LICENSE-2.0
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
    AlreadyInitialized,
    InvalidWiFiMode
};

class CaptivePortal {
public:
    bool initializeOpen(const char* ssid, const char* defaultFile = "index.html", WiFiMode_t mode = WIFI_AP);
    bool initialize(const char* ssid, const char* password, const char* defaultFile = "index.html", WiFiMode_t mode = WIFI_AP);
    bool stopAP();
    bool startAP();
    bool processDNS();
    AsyncWebServer& getServer();
    CaptivePortalError getLastError() const;
    String getLastErrorString() const;

private:
    void registerRoutes();

    AsyncWebServer server = AsyncWebServer(80);
    DNSServer dnsServer;
    bool apRunning = false;
    bool initialized = false;
    CaptivePortalError lastError = CaptivePortalError::None;
};
