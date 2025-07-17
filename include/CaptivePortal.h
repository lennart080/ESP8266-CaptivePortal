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
    InvalidWiFiMode,
    WebSocketNotInitialized
};

const std::unordered_map<CaptivePortalError, String> CaptivePortalErrorStrings = {
    {CaptivePortalError::None, "No error: Operation completed successfully."},
    {CaptivePortalError::AlreadyRunning, "Captive portal is already running: Cannot start a new instance while one is active."},
    {CaptivePortalError::FSInitFailed, "Filesystem initialization failed: Unable to mount or access the filesystem (LittleFS)."},
    {CaptivePortalError::APStartFailed, "Access Point start failed: Could not configure or enable the WiFi access point."},
    {CaptivePortalError::DNSServerStartFailed, "DNS server start failed: Unable to initialize or start the DNS server for captive portal redirection."},
    {CaptivePortalError::NotRunning, "Captive portal is not running: Attempted operation requires the portal to be active."},
    {CaptivePortalError::Unknown, "Unknown error: An unspecified or unexpected error occurred."},
    {CaptivePortalError::FileNotFound, "Default file not found: The specified default file (e.g., index.html) does not exist in the filesystem."},
    {CaptivePortalError::SoftAPDisconnectFailed, "SoftAP disconnect failed: Unable to properly stop or disconnect the WiFi access point."},
    {CaptivePortalError::InvalidSSID, "Invalid SSID: The provided SSID is empty, too long, or contains invalid characters."},
    {CaptivePortalError::InvalidPassword, "Invalid password: The provided WiFi password does not meet security requirements."},
    {CaptivePortalError::NotInitialized, "Not initialized: The captive portal must be initialized before performing this operation."},
    {CaptivePortalError::AlreadyInitialized, "Already initialized: The captive portal has already been initialized and cannot be initialized again."},
    {CaptivePortalError::InvalidWiFiMode, "Invalid WiFi mode: The specified WiFi mode is not supported for captive portal operation."},
    {CaptivePortalError::WebSocketNotInitialized, "WebSocket not initialized: Attempted to access or use the WebSocket before it was set up."}
};

class CaptivePortal {
public:
    bool initializeOpen(const char* ssid, const char* defaultFile = "index.html", WiFiMode_t mode = WIFI_AP, bool addWebSocket = false);
    bool initialize(const char* ssid, const char* password, const char* defaultFile = "index.html", WiFiMode_t mode = WIFI_AP, bool addWebSocket = false);
    bool stopAP();
    bool startAP();
    bool processDNS();
    AsyncWebServer& getServer();
    AsyncWebSocket& getWebSocket();
    CaptivePortalError getLastError() const;
    String getLastErrorString() const;

private:
    void registerRoutes();

    AsyncWebServer server = AsyncWebServer(80);
    DNSServer dnsServer;
    AsyncWebSocket* ws = nullptr;
    bool apRunning = false;
    bool initialized = false;
    CaptivePortalError lastError = CaptivePortalError::None;
};
