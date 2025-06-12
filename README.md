# CaptivePortal
[![License](https://img.shields.io/github/license/lennart080/ESP8266-CaptivePortal?color=blue)](https://github.com/lennart080/ESP8266-CaptivePortal/blob/main/LICENSE)
[![PlatformIO Registry](https://badgen.net/badge/PlatformIO/Available/green?icon=platformio)](https://registry.platformio.org/libraries/lennart080/ESP8266-CaptivePortal)
[![GitHub Release](https://img.shields.io/github/v/release/lennart080/ESP8266-CaptivePortal)](https://github.com/lennart080/ESP8266-CaptivePortal/releases)

A lightweight captive portal library for ESP8266 (Arduino/PlatformIO).  
Lets users connect to a temporary WiFi access point, open a web portal, and interact with your device (e.g., submit WiFi credentials or other data).

---

## Features

- **Captive portal:** Redirects all DNS and HTTP traffic to the ESP for easy setup.
- **Static file serving:** Serves files (e.g., `index.html`, CSS, JS) from LittleFS.
- **Custom API endpoints:** Easily add your own HTTP handlers (e.g., `/api/wifi`).
- **Captive portal detection:** Handles Android, Windows, and Apple captive portal checks.
- **Simple API:** Minimal setup, easy to extend.

---

## Getting Started

### Hardware Requirements

- ESP8266-based board (e.g., NodeMCU, Wemos D1 Mini)
- PlatformIO or Arduino IDE

### Dependencies

- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) <br>
  <sub>Recommended: <strong>^3.6.0</strong></sub>
- [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) <br>
  <sub>Recommended: <strong>^2.0.0</strong></sub>
- [LittleFS](https://github.com/earlephilhower/arduino-esp8266littlefs) <br>
  <sub>Recommended: <strong>^2.11.0</strong></sub>
- [DNSServer](https://arduino-esp8266.readthedocs.io/en/latest/libraries.html#dns-server) <br>
  <sub>Included with ESP8266 Arduino core</sub>

Install via PlatformIO `lib_deps` or Arduino Library Manager.

---

## Installation

### PlatformIO

Add this library to your `platformio.ini`:
```
lib_deps =
    https://github.com/lennart080/ESP8266-CaptivePortal.git
    # Optional: Only add the following if PlatformIO cannot find ESPAsyncWebServer automatically
    me-no-dev/ESPAsyncWebServer
```
> **Note:**  
> The `me-no-dev/ESPAsyncWebServer` dependency is optional and should only be added if PlatformIO fails to resolve it automatically and you encounter an error about a missing library.

Or install via the PlatformIO Library Manager.

### Arduino IDE

1. Download this repository as a ZIP file.
2. In Arduino IDE, go to **Sketch > Include Library > Add .ZIP Library...** and select the downloaded ZIP.
3. Install the required dependencies:
    - [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
    - [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP)
    - [LittleFS](https://github.com/earlephilhower/arduino-esp8266littlefs)

### LittleFS Data Upload

To serve web files (like `index.html`), upload them to the ESP8266's LittleFS filesystem.

- **PlatformIO:**  
  LittleFS upload is natively supported. Add to your `platformio.ini`:
  ```
  board_build.filesystem = littlefs
  ```
  Place your web files in the `/data` folder. Then run:
  ```
  pio run --target uploadfs
  ```

- **Arduino IDE:**  
  Use the [LittleFS Data Upload tool](https://github.com/earlephilhower/arduino-esp8266littlefs#uploading-files-to-filesystem).

---

## Usage

### 1. Include and Create

```cpp
#include "CaptivePortal.h"
CaptivePortal portal;
```

### 2. Initialize and Start the Portal

You can initialize the portal as an open AP (no password):

```cpp
if (!portal.initializeOpen("My-Example-AP", "index.html")) {
    Serial.println("Init failed: " + portal.getLastErrorString());
}
```

Or as a password-protected AP:

```cpp
if (!portal.initialize("My-Example-AP", "12345678", "index.html")) {
    Serial.println("Init failed: " + portal.getLastErrorString());
}
```
- The password must be 8–63 characters for WPA2 AP mode.
- The third argument is the default file to serve (e.g., `"index.html"` from LittleFS).

Start the portal:

```cpp
if (!portal.startAP()) {
    Serial.println("Start failed: " + portal.getLastErrorString());
}
```

### 3. Main Loop

Call `processDNS()` in your loop to handle DNS redirection:

```cpp
void loop() {
    portal.processDNS();
    delay(100);
}
```

### 4. Adding Custom API Endpoints

You can add your own HTTP handlers using the underlying AsyncWebServer:

```cpp
portal.getServer().on("/api", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("text", true)) {
        String text = request->getParam("text", true)->value();
        // Save credentials, connect, etc.
        request->send(200, "text/plain", "Text received.");
        portal.stopAP();
    } else {
        request->send(400, "text/plain", "Missing Text.");
    }
});
```

#### Example `script.js` for Sending Data to `/api`

Here’s a simple example of how you could send data from your frontend to the custom API endpoint using JavaScript:

```js
function sendText() {
    const text = document.getElementById('textInput').value;
    fetch('/api', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: 'text=' + encodeURIComponent(text)
    })
    .then(response => response.text())
    .then(data => {
        alert('Server response: ' + data);
    })
    .catch(error => {
        alert('Error: ' + error);
    });
}
```

And in your `index.html`:

```html
<input id="textInput" type="text" placeholder="Enter text">
<button onclick="sendText()">Send</button>
<script src="script.js"></script>
```

- This example sends the value from an input field to your `/api` endpoint using a POST request.
- Adjust the parameter name and logic as needed for your use case.

### 5. Serving the Web Interface

- Place your `index.html`, `style.css`, and `script.js` in the device's LittleFS filesystem.
- The portal will serve these files automatically at `/`.

---

## Troubleshooting

- **LittleFS mount failed:** Ensure you have uploaded files to LittleFS and the filesystem is formatted.
- **File not found:** Make sure your default file (e.g., `index.html`) exists in LittleFS.
- **Invalid SSID/Password:** SSID must be 1–32 chars; password must be 8–63 chars or empty for open AP.
- **AP won't start:** Check for conflicting WiFi modes or hardware issues.

---

## API Reference

| Method                                         | Description                                      |
|------------------------------------------------|--------------------------------------------------|
| `bool initializeOpen(ssid, defaultFile)`       | Open AP, no password.                            |
| `bool initialize(ssid, password, defaultFile)`  | WPA2 AP, with password.                          |
| `bool startAP()`                               | Start the AP and web server.                     |
| `bool stopAP()`                                | Stop the AP and web server.                      |
| `void processDNS()`                            | Handle DNS requests (call in loop).              |
| `AsyncWebServer& getServer()`                  | Access the underlying AsyncWebServer.            |
| `CaptivePortalError getLastError() const`      | Get the last error code.                         |
| `String getLastErrorString() const`            | Get a string describing the last error.          |

---

## License

This project is licensed under the Apache 2.0 License.

You may use, share and modify it, as long as you give credit to:

**Lennart Gutjahr (2025)** — Original author  
**gutjahrlennart@gmail.com** — Email

License details: [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0)

---

## Credits

Based on [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) and [LittleFS](https://github.com/earlephilhower/arduino-esp8266littlefs).
