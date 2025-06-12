#include <Arduino.h>
#include "CaptivePortal.h"

// Example: Minimal captive portal usage.
// Before running, upload your web files (e.g., index.html) to LittleFS.

CaptivePortal portal;

void setup() {
    Serial.begin(115200);

    // Initialize the portal with SSID, password, and default file.
    if (!portal.initialize("My-Example-AP", "12345678", "index.html")) {
        Serial.println("Init error: " + portal.getLastErrorString());
        return;
    }
    
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
//Example index.html for basic captive portal usage:
// Copy and include this in your index.html file

<!DOCTYPE html>
<html>
<head>
    <title>Captive Portal Example</title>
    <script src="script.js"></script>
</head>
<body>
    <h1>Welcome to the Captive Portal</h1>
    <p>This is a basic example of a captive portal.</p>
</body>
</html>
*/
