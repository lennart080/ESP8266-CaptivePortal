#include <Arduino.h>
#include "CaptivePortal.h"

// Example: Custom API handler that receives text and stops the portal.
// Before running, upload your web files (e.g., index.html) to LittleFS.
// script.js example is provided in comments at the Bottom.

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

/*
Example script.js/index.html for interacting with /api endpoint:

// Copy and include this in your script.js file
function sendText() {
    fetch('/api', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: 'text=' + encodeURIComponent(document.getElementById('myText').value)
    })
    .then(response => response.text())
    .then(data => alert(data))
    .catch(err => alert('Error: ' + err));
}

// Example HTML:
// <input id="myText" type="text" placeholder="Enter text">
// <button onclick="sendText()">Send</button>
*/
