#include "WebPortal.h"
#include <Arduino.h>

// Before running, upload your web files (e.g., index.html) to LittleFS using the LittleFS Data Upload tool.

WebPortal portal;

void setup() {
    portal.initialize("My-Example-AP", "12345678", "index.html");

    portal.getServer().on("/api", HTTP_POST, [&](AsyncWebServerRequest *request) {
        if (request->hasParam("text", true)) {
            String receivedText = request->getParam("text", true)->value();

            request->send(200, "text/plain", "Text received. Portal will close.");
            portal.stopAP();
        } else {
            request->send(400, "text/plain", "Missing Text");
        }
    });

    portal.startAP();
}

void loop() {
    portal.processDNS();
    delay(100);
}