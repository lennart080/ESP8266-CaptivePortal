#include <Arduino.h>
#include "CaptivePortal.h"

// Example: Captive portal with a shutdown endpoint.
// Before running, upload your web files (e.g., index.html) to LittleFS.
// script.js example is provided in comments at the Bottom.

CaptivePortal portal;

void setup() {
    Serial.begin(115200);

    // Initialize the portal.
    if (!portal.initialize("Advanced-Portal", "87654321", "index.html")) {
        Serial.println("Init error: " + portal.getLastErrorString());
        return;
    }

    // Custom POST endpoint: /shutdown
    portal.getServer().on("/shutdown", HTTP_POST, [&](AsyncWebServerRequest *request) {
        if (!portal.stopAP()) {
            Serial.println("Stop AP error: " + portal.getLastErrorString());
            request->send(500, "text/plain", "Portal stop error: " + portal.getLastErrorString());
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

/*
Example script.js/index.html for interacting with /shutdown endpoint:

// Copy and include this in your script.js file
function shutdownPortal() {
    fetch('/shutdown', {
        method: 'POST'
    })
    .then(response => response.text())
    .then(data => alert(data))
    .catch(err => alert('Error: ' + err));
}

// Example HTML:
// <button onclick="shutdownPortal()">Shutdown Portal</button>
*/
