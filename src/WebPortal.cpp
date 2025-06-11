// Licensed under CC BY-NC 4.0
// https://creativecommons.org/licenses/by-nc/4.0/
// © 2025 Lennart Gutjahr

#include "WebPortal.h"

const byte DNS_PORT = 53;

void WebPortal::processDNS() {
    dnsServer.processNextRequest();
}

AsyncWebServer& WebPortal::getServer() {
    return server;
}

void WebPortal::registerRoutes(const char* defaultFile) {
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

bool WebPortal::startAP() {
    if (apRunning) {
        return false;
    }

    if (!LittleFS.begin()) {
        return false;
    }

    server.begin();
    apRunning = true;
    return true;
}

bool WebPortal::initialize(const char* ssid, const char* password, const char* defaultFile) {
    if (apRunning) {
        return false;
    }

    if (!LittleFS.begin()) {
        return false;
    }

    WiFi.mode(WIFI_AP);

    if (password && strlen(password) >= 8) {
        WiFi.softAP(ssid, password);
    } else {
        WiFi.softAP(ssid);
    }

    IPAddress myIP = WiFi.softAPIP();
    dnsServer.start(DNS_PORT, "*", myIP);

    registerRoutes(defaultFile);

    server.begin();
    apRunning = true;
    return true;
}

bool WebPortal::stopAP() {
    if (apRunning) {
        server.end();
        WiFi.softAPdisconnect(true);
        LittleFS.end();
        apRunning = false;
        return true;
    } 
    return false;
}
