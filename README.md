# CaptivePortal

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

## Folder Structure

```
lib/CaptivePortal/
  README.md
  LICENSE
  library.json
  library.properties
  examples/
    BasicUsage/
      BasicUsage.ino
    GetText/
      GetText.ino
  src/
    CaptivePortal.h
    CaptivePortal.cpp
```

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

Call `initialize()` in `setup()` to configure the AP and web server, then call `startAP()` to start the portal.

```cpp
void setup() {
    portal.initialize("My-Example-AP", "12345678", "index.html");
    portal.startAP();
}
```
- The password must be at least 8 characters for WPA2 AP mode.
- The third argument is the default file to serve (e.g., `"index.html"` from LittleFS).

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
// Example script.js
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
- **Note:** In your `script.js`, you are responsible for writing the JavaScript code to interact with your custom REST API endpoints (e.g., using `fetch` or `XMLHttpRequest` to POST data to `/api`). The library does not generate frontend code for you.

---

## Example

**Basic usage:**

```cpp
#include "CaptivePortal.h"
CaptivePortal portal;

void setup() {
    portal.initialize("My-Example-AP", "12345678", "index.html");
    portal.startAP();
}

void loop() {
    portal.processDNS();
    delay(100);
}
```

See [`examples/BasicUsage/BasicUsage.ino`](examples/BasicUsage/BasicUsage.ino) and [`examples/GetText/GetText.ino`](examples/GetText/GetText.ino) for more.

---

## API Reference

| Method                                      | Description                                      |
|----------------------------------------------|--------------------------------------------------|
| `bool initialize(ssid, password, defaultFile)` | Configure AP, DNS, and web server.               |
| `bool startAP()`                             | Start the AP and web server.                     |
| `bool stopAP()`                              | Stop the AP and web server.                      |
| `void processDNS()`                          | Handle DNS requests (call in loop).              |
| `AsyncWebServer& getServer()`                | Access the underlying AsyncWebServer.            |

**Note:**  
- There is no built-in credential storage or WiFi connection logic; you handle this via your own API endpoints and handlers.
- The portal does not automatically close; call `stopAP()` when you want to shut it down.

---

## Customization

- **Static files:** Place your web assets in LittleFS (`/data` folder for PlatformIO).
- **Routes:** Add custom HTTP handlers using `portal.getServer().on(...)` before calling `startAP()`.
- **Captive portal detection:** Handles `/generate_204`, `/ncsi.txt`, `/fwlink`, `/hotspot-detect.html`, and `/captive.apple.com` for compatibility.

---

## Notes

- All DNS queries are redirected to the ESP, ensuring captive portal detection on most devices.
- The portal will only close after you call `stopAP()`.

---

## License

This project is licensed under the Creative Commons Attribution-NonCommercial 4.0 License.

You may use, share, and modify it for non-commercial purposes, as long as you give credit to:

**Lennart Gutjahr (2025)** — Original author  
**gutjahrlennart@gmail.com** — Email

License details: https://creativecommons.org/licenses/by-nc/4.0/

---

## Credits

- Based on [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) and [LittleFS](https://github.com/earlephilhower/arduino-esp8266littlefs).
