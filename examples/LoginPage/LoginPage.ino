#include <Arduino.h>
#include "CaptivePortal.h"

// Example: Custom login page with POST handler.
// Before running, upload your web files (e.g., login.html) to LittleFS.
// script.js example is provided in comments at the Bottom.

CaptivePortal portal;

void setup() {
    Serial.begin(115200);

    // Initialize the portal with a custom login page.
    if (!portal.initialize("Login-Portal", "supersecret", "login.html")) {
        Serial.println("Init error: " + portal.getLastErrorString());
        return;
    }

    // Custom POST endpoint: /login
    portal.getServer().on("/login", HTTP_POST, [&](AsyncWebServerRequest *request) {
        // Check for required parameters
        if (request->hasParam("username", true) && request->hasParam("password", true)) {
            String user = request->getParam("username", true)->value();
            String pass = request->getParam("password", true)->value();

            // Simple authentication (replace with your logic)
            if (user == "admin" && pass == "admin") {
                request->send(200, "text/plain", "Login successful!");
            } else {
                request->send(401, "text/plain", "Invalid credentials");
            }
        } else {
            request->send(400, "text/plain", "Missing parameters");
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
Example script.js/login.html for interacting with /login endpoint:

// Copy and include this in your script.js file
function login() {
    fetch('/login', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: 'username=' + encodeURIComponent(document.getElementById('user').value) +
              '&password=' + encodeURIComponent(document.getElementById('pass').value)
    })
    .then(response => response.text())
    .then(data => alert(data))
    .catch(err => alert('Error: ' + err));
}

// Example HTML:
// <input id="user" type="text" placeholder="Username">
// <input id="pass" type="password" placeholder="Password">
// <button onclick="login()">Login</button>
*/
