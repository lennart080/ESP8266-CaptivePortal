// Licensed under Apache License, Version 2.0
// SPDX-License-Identifier: Apache-2.0
// http://www.apache.org/licenses/LICENSE-2.0
// © 2025 Lennart Gutjahr

#include "CaptivePortal.h"

const byte DNS_PORT = 53;

/**
 * @brief Processes the next DNS request.
 * 
 * This method processes incoming DNS requests and redirects them to the captive portal's IP address.
 * It should be called frequently to ensure that devices can resolve the captive portal's hostname.
 */
bool CaptivePortal::processDNS() {
    if (!initialized) {
        lastError = CaptivePortalError::NotInitialized;
        return false;
    }
    if (!apRunning) {
        lastError = CaptivePortalError::NotRunning;
        return false;
    }
    lastError = CaptivePortalError::None;
    dnsServer.processNextRequest();
    return true;
}

/**
 * @brief Get a reference to the internal AsyncWebServer.
 * 
 * @return Reference to the AsyncWebServer instance.
 * 
 * This method allows you to access the internal server instance
 * to add custom routes or handlers beyond the captive portal's default functionality.
 */
AsyncWebServer& CaptivePortal::getServer() {
    if (!initialized) {
        lastError = CaptivePortalError::NotInitialized;
        return server;
    }
    lastError = CaptivePortalError::None;
    return server;
}

/**
 * @brief Registers the default and captive portal routes.
 * 
 * This method sets up the routes for the captive portal, including handling
 * common requests that devices make when they detect a captive portal.
 * It includes routes for Android, Windows, and iOS captive portal checks.
 */
void CaptivePortal::registerRoutes() {
    server.onNotFound([this](AsyncWebServerRequest *request) {
        request->redirect("/");  // Redirect all unknown requests to the root
    });

    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(204, "text/plain", "");  // Captive portal android check
    });

    server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Microsoft NCSI");  // Captive portal Windows check
    });

    server.on("/fwlink", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/");  // Redirect fwlink requests to the root
    });

    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");  // Captive portal iOS check
    });

    server.on("/captive.apple.com", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");  // Captive portal iOS check
    });
}

/**
 * @brief Get the last error that occurred.
 * 
 * @return The last CaptivePortalError.
 * 
 * This method returns the last error that occurred during operations
 * such as initialization, starting the access point, or processing requests.
 * When a Funktion returns false, you can call this method to get more information about the failure.
 */
CaptivePortalError CaptivePortal::getLastError() const {
    return lastError;
}

/**
 * @brief Get a string representation of the last error.
 * 
 * @return A string describing the last error.
 * 
 * This method provides a human-readable description of the last error that occurred.
 * It can be useful for debugging or logging purposes.
 */
String CaptivePortal::getLastErrorString() const {
    return String(static_cast<int>(lastError));
}

/**
 * @brief Start the access point if it is initialized but not running.
 * 
 * @return true if the AP was started successfully, false otherwise.
 * 
 * This method starts the access point if it has been initialized and is not already running.
 * It sets up the DNS server and begins serving requests.
 */
bool CaptivePortal::startAP() {
    if (apRunning) {
        lastError = CaptivePortalError::AlreadyRunning;
        return false;
    }

    if (!initialized) {
        lastError = CaptivePortalError::NotInitialized;
        return false;
    }

    server.begin();
    apRunning = true;
    lastError = CaptivePortalError::None;
    return true;
}

/**
 * @brief Initialize the captive portal as an open access point (no password).
 * 
 * @param ssid The SSID for the access point.
 * @param defaultFile The default file to serve (e.g., "index.html").
 * @param mode The WiFi mode to use (default is WIFI_AP).
 * @return true if initialization succeeded, false otherwise.
 * 
 * This method initializes the captive portal with the specified SSID and default file.
 * It sets up the access point without a password and prepares the DNS server to redirect requests.
 * The default file is the HTML file that will be served when a device connects to the captive portal, it should exist in the LittleFS filesystem.
 */
bool CaptivePortal::initializeOpen(const char* ssid, const char* defaultFile, WiFiMode_t mode, bool addWebSocket) {
    return initialize(ssid, nullptr, defaultFile, mode, addWebSocket);
}

/**
 * @brief Initialize the captive portal as a password-protected or open access point.
 * 
 * @param ssid The SSID for the access point.
 * @param password The password for the access point.
 * @param defaultFile The default file to serve (e.g., "index.html").
 * @param mode The WiFi mode to use (default is WIFI_AP).
 * @return true if initialization succeeded, false otherwise.
 * 
 * This method initializes the captive portal with the specified SSID, password, and default file.
 * It sets up the access point with the given SSID and password and prepares the DNS server to redirect requests.
 * The default file is the HTML file that will be served when a device connects to the captive portal, it should exist in the LittleFS filesystem.
 */
bool CaptivePortal::initialize(const char* ssid, const char* password, const char* defaultFile, WiFiMode_t mode, bool addWebSocket) {

    if (!ssid || strlen(ssid) == 0 || strlen(ssid) > 32) {
        lastError = CaptivePortalError::InvalidSSID; 
        return false;
    }

    if (password) {
        size_t len = strlen(password);
        if (len < 8 || len > 63) {
            lastError = CaptivePortalError::InvalidPassword; 
            return false;
        }
    }

    if (initialized) {
        lastError = CaptivePortalError::AlreadyInitialized;
        return false;
    }

    if (apRunning) {
        lastError = CaptivePortalError::AlreadyRunning;
        return false;
    }

    if (!LittleFS.begin()) {
        lastError = CaptivePortalError::FSInitFailed;
        return false;
    }

    if (!defaultFile || !LittleFS.exists(String("/") + defaultFile)) {
        lastError = CaptivePortalError::FileNotFound;
        LittleFS.end();
        return false;
    }

    if (mode != WIFI_AP && mode != WIFI_AP_STA) {
        lastError = CaptivePortalError::InvalidWiFiMode;
        LittleFS.end();
        return false;
    }
    WiFi.mode(mode);

    bool apResult;
    if (password) {
        apResult = WiFi.softAP(ssid, password);
    } else {
        apResult = WiFi.softAP(ssid);
    }

    if (!apResult) {
        lastError = CaptivePortalError::APStartFailed;
        LittleFS.end();
        return false;
    }

    IPAddress myIP = WiFi.softAPIP();
    if (!dnsServer.start(DNS_PORT, "*", myIP)) {
        lastError = CaptivePortalError::DNSServerStartFailed;
        WiFi.softAPdisconnect(true);
        LittleFS.end();
        return false;
    }

    server.serveStatic("/", LittleFS, "/").setDefaultFile(defaultFile);

    registerRoutes();

    if (addWebSocket) {
        ws = new AsyncWebSocket("/ws");
        server.addHandler(ws);
    }

    server.begin();
    initialized = true;
    lastError = CaptivePortalError::None;
    return true;
}

/**
 * @brief Stop the access point and clean up resources.
 * 
 * @return true if the AP was stopped successfully, false otherwise.
 * 
 * This method stops the access point, ends the DNS server, and cleans up the LittleFS filesystem.
 * It should be called when the captive portal is no longer needed.
 */
bool CaptivePortal::stopAP() {
    if (!apRunning) {
        lastError = CaptivePortalError::NotRunning;
        return false;
    }
    server.end();
    dnsServer.stop();
    if (!WiFi.softAPdisconnect(true)) {
        lastError = CaptivePortalError::SoftAPDisconnectFailed;
        return false;
    }
    LittleFS.end();

    if (ws != nullptr) {
        delete ws;
        ws = nullptr;
    }

    apRunning = false;
    initialized = false;
    lastError = CaptivePortalError::None;
    return true;
}

/**
 * @brief Get a reference to the internal AsyncWebSocket.
 * 
 * @return Reference to the AsyncWebSocket instance.
 * 
 * This method allows you to access the internal WebSocket instance
 * to add custom handlers or manage WebSocket connections. If the WebSocket is not initialized or is null, it returns a dummy instance
 * and sets the last error to `WebSocketNotInitialized`. 
 */
AsyncWebSocket& CaptivePortal::getWebSocket() {
    if (!initialized || ws == nullptr) {
        lastError = CaptivePortalError::WebSocketNotInitialized;
        static AsyncWebSocket dummy("/dummy");
        return dummy;
    }
    lastError = CaptivePortalError::None;
    return *ws;
}
