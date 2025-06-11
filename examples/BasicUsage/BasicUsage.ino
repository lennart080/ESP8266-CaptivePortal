#include "WebPortal.h"
#include <Arduino.h>

// Before running, upload your web files (e.g., index.html) to LittleFS using the LittleFS Data Upload tool.

WebPortal portal;

void setup() {
    portal.initialize("My-Example-AP", "12345678", "index.html");
    portal.startAP();
}

void loop() {
    portal.processDNS();
    delay(100);
}