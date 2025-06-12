#include <Arduino.h>
#include "CaptivePortal.h"

// Example: Custom API handler that receives text and stops the portal.
// Before running, upload your web files (e.g., index.html) to LittleFS.

CaptivePortal portal;

void setup() {
    Serial.begin(115200);

    // Initialize the portal.
    if (!portal.initialize("My-Example-AP", "12345678", "index.html")) {
        Serial.println("Init error: " + portal.getLastErrorString());
        return;
    }

    // Custom POST endpoint: /api
    portal.getServer().on("/api", HTTP_POST, [&](AsyncWebServerRequest *request) {
        if (request->hasParam("text", true)) {
            String receivedText = request->getParam("text", true)->value();
            // Respond and stop the portal
            request->send(200, "text/plain", "Text received. Portal will close.");
            if (!portal.stopAP()) {
                Serial.println("Failed to stop portal: " + portal.getLastErrorString());
            }
        } else {
            request->send(400, "text/plain", "Missing Text");
        }
    });

    // Start the access point.
    if (!portal.startAP()) {
        Serial.println("Start AP error: " + portal.getLastErrorString());
        return;
    }
}

void loop() {
    // Must be called frequently to handle DNS requests.
    portal.processDNS();
    delay(100);
}