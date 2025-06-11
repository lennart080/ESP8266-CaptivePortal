#include <Arduino.h>
#include "CaptivePortal.h"

// Before running, upload your web files (e.g., index.html) to LittleFS using the LittleFS Data Upload tool.

CaptivePortal portal;

void setup() {
    Serial.begin(115200);

    if (!portal.initialize("My-Example-AP", "12345678", "index.html")) {
        Serial.println(String(static_cast<int>(portal.getLastError())));
        return;
    }

    portal.getServer().on("/api", HTTP_POST, [&](AsyncWebServerRequest *request) {
        if (request->hasParam("text", true)) {
            String receivedText = request->getParam("text", true)->value();

            request->send(200, "text/plain", "Text received. Portal will close.");
            portal.stopAP();
        } else {
            request->send(400, "text/plain", "Missing Text");
        }
    });

    if (!portal.startAP()) {
        Serial.println(String(static_cast<int>(portal.getLastError())));
        return;
    }
}

void loop() {
    portal.processDNS();
    delay(100);
}