// Licensed under CC BY-NC 4.0
// https://creativecommons.org/licenses/by-nc/4.0/
// © 2025 Lennart Gutjahr

#include "CaptivePortal.h"

const byte DNS_PORT = 53;

void CaptivePortal::processDNS() {
    dnsServer.processNextRequest();
}

AsyncWebServer& CaptivePortal::getServer() {
    return server;
}

void CaptivePortal::registerRoutes(const char* defaultFile) {
    server.serveStatic("/", LittleFS, "/").setDefaultFile(defaultFile);

    server.onNotFound([this](AsyncWebServerRequest *request) {
        request->redirect("/");
    });

    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(204, "text/plain", "");
    });

    server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Microsoft NCSI");
    });

    server.on("/fwlink", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/");
    });

    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
    });

    server.on("/captive.apple.com", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
    });
}

CaptivePortalError CaptivePortal::getLastError() const {
    return lastError;
}

bool CaptivePortal::startAP() {
    if (apRunning) {
        lastError = CaptivePortalError::AlreadyRunning;
        return false;
    }

    if (!initialized) {
        lastError = CaptivePortalError::NotInitialized;
        return false;
    }

    server.begin();
    apRunning = true;
    lastError = CaptivePortalError::None;
    return true;
}

bool CaptivePortal::initialize(const char* ssid, const char* defaultFile) {
    return initialize(ssid, nullptr, defaultFile);
}

bool CaptivePortal::initialize(const char* ssid, const char* password, const char* defaultFile) {

    if (!ssid || strlen(ssid) == 0 || strlen(ssid) > 32) {
        lastError = CaptivePortalError::InvalidSSID; 
        return false;
    }

    if (password) {
        size_t len = strlen(password);
        if (len < 8 || len > 63) {
            lastError = CaptivePortalError::InvalidPassword; 
            return false;
        }
    }

    if (initialized) {
        lastError = CaptivePortalError::AlreadyInitialized;
        return false;
    }

    if (apRunning) {
        lastError = CaptivePortalError::AlreadyRunning;
        return false;
    }

    if (!LittleFS.begin()) {
        lastError = CaptivePortalError::FSInitFailed;
        return false;
    }

    if (!defaultFile || !LittleFS.exists(String("/") + defaultFile)) {
        lastError = CaptivePortalError::FileNotFound;
        LittleFS.end();
        return false;
    }

    WiFi.mode(WIFI_AP);

    bool apResult;
    if (password) {
        apResult = WiFi.softAP(ssid, password);
    } else {
        apResult = WiFi.softAP(ssid);
    }

    if (!apResult) {
        lastError = CaptivePortalError::APStartFailed;
        LittleFS.end();
        return false;
    }

    IPAddress myIP = WiFi.softAPIP();
    if (!dnsServer.start(DNS_PORT, "*", myIP)) {
        lastError = CaptivePortalError::DNSServerStartFailed;
        WiFi.softAPdisconnect(true);
        LittleFS.end();
        return false;
    }

    registerRoutes(defaultFile);

    server.begin();
    initialized = true;
    lastError = CaptivePortalError::None;
    return true;
}

bool CaptivePortal::stopAP() {
    if (!apRunning) {
        lastError = CaptivePortalError::NotRunning;
        return false;
    }
    server.end();
    dnsServer.stop();
    if (!WiFi.softAPdisconnect(true)) {
        lastError = CaptivePortalError::SoftAPDisconnectFailed;
        return false;
    }
    LittleFS.end();
    apRunning = false;
    initialized = false;
    lastError = CaptivePortalError::None;
    return true;
}
