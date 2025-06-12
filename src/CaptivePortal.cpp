// Licensed under CC BY-NC 4.0
// https://creativecommons.org/licenses/by-nc/4.0/
// © 2025 Lennart Gutjahr

#include "CaptivePortal.h"

const byte DNS_PORT = 53;

/**
 * @brief Processes the next DNS request.
 * 
 * This method processes incoming DNS requests and redirects them to the captive portal's IP address.
 * It should be called frequently to ensure that devices can resolve the captive portal's hostname.
 */
void CaptivePortal::processDNS() {
    dnsServer.processNextRequest();
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
 * @return true if initialization succeeded, false otherwise.
 * 
 * This method initializes the captive portal with the specified SSID and default file.
 * It sets up the access point without a password and prepares the DNS server to redirect requests.
 * The default file is the HTML file that will be served when a device connects to the captive portal, it should exist in the LittleFS filesystem.
 */
bool CaptivePortal::initialize(const char* ssid, const char* defaultFile) {
    return initialize(ssid, nullptr, defaultFile);
}

/**
 * @brief Initialize the captive portal as a password-protected or open access point.
 * 
 * @param ssid The SSID for the access point.
 * @param password The password for the access point.
 * @param defaultFile The default file to serve (e.g., "index.html").
 * @return true if initialization succeeded, false otherwise.
 * 
 * This method initializes the captive portal with the specified SSID, password, and default file.
 * It sets up the access point with the given SSID and password and prepares the DNS server to redirect requests.
 * The default file is the HTML file that will be served when a device connects to the captive portal, it should exist in the LittleFS filesystem.
 */
bool CaptivePortal::initialize(const char* ssid, const char* password, const char* defaultFile) {

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

    WiFi.mode(WIFI_AP);

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
    apRunning = false;
    initialized = false;
    lastError = CaptivePortalError::None;
    return true;
}
