#include <Arduino.h>
#include <unity.h>
#include "CaptivePortal.h"

// Test: SSID is null
void test_invalid_ssid_null() {
    CaptivePortal portal;
    TEST_ASSERT_FALSE(portal.initialize(nullptr, "12345678", "index.html"));
    TEST_ASSERT_EQUAL(CaptivePortalError::InvalidSSID, portal.getLastError());
}

// Test: SSID is empty
void test_invalid_ssid_empty() {
    CaptivePortal portal;
    TEST_ASSERT_FALSE(portal.initialize("", "12345678", "index.html"));
    TEST_ASSERT_EQUAL(CaptivePortalError::InvalidSSID, portal.getLastError());
}

// Test: SSID too long (>32 chars)
void test_invalid_ssid_too_long() {
    CaptivePortal portal;
    const char* long_ssid = "123456789012345678901234567890123"; // 33 chars
    TEST_ASSERT_FALSE(portal.initialize(long_ssid, "12345678", "index.html"));
    TEST_ASSERT_EQUAL(CaptivePortalError::InvalidSSID, portal.getLastError());
}

// Test: Password too short (<8 chars)
void test_invalid_password_too_short() {
    CaptivePortal portal;
    TEST_ASSERT_FALSE(portal.initialize("TestAP", "123", "index.html"));
    TEST_ASSERT_EQUAL(CaptivePortalError::InvalidPassword, portal.getLastError());
}

// Test: Password too long (>63 chars)
void test_invalid_password_too_long() {
    CaptivePortal portal;
    char long_pw[65];
    memset(long_pw, 'a', 64);
    long_pw[64] = '\0';
    TEST_ASSERT_FALSE(portal.initialize("TestAP", long_pw, "index.html"));
    TEST_ASSERT_EQUAL(CaptivePortalError::InvalidPassword, portal.getLastError());
}

// Test: File not found
void test_file_not_found() {
    CaptivePortal portal;
    TEST_ASSERT_FALSE(portal.initialize("TestAP", "12345678", "notfound.html"));
    TEST_ASSERT_EQUAL(CaptivePortalError::FileNotFound, portal.getLastError());
}

// Test: Already running
void test_already_running() {
    CaptivePortal portal;
    // Assume "index.html" exists in LittleFS for this test to pass
    TEST_ASSERT_TRUE(portal.initialize("TestAP", "12345678", "index.html"));
    TEST_ASSERT_TRUE(portal.startAP());
    TEST_ASSERT_FALSE(portal.startAP());
    TEST_ASSERT_EQUAL(CaptivePortalError::AlreadyRunning, portal.getLastError());
    portal.stopAP();
}

// Test: Not initialized
void test_not_initialized() {
    CaptivePortal portal;
    TEST_ASSERT_FALSE(portal.startAP());
    TEST_ASSERT_EQUAL(CaptivePortalError::NotInitialized, portal.getLastError());
}

// Test: Stop AP when not running
void test_stop_ap_not_running() {
    CaptivePortal portal;
    TEST_ASSERT_FALSE(portal.stopAP());
    TEST_ASSERT_EQUAL(CaptivePortalError::NotRunning, portal.getLastError());
}

void test_double_initialize() {
    CaptivePortal portal;
    // Assume "index.html" exists in LittleFS
    TEST_ASSERT_TRUE(portal.initialize("TestAP", "12345678", "index.html"));
    TEST_ASSERT_FALSE(portal.initialize("TestAP", "12345678", "index.html"));
    TEST_ASSERT_EQUAL(CaptivePortalError::AlreadyInitialized, portal.getLastError());
    portal.stopAP();
}

void test_stop_ap_after_start() {
    CaptivePortal portal;
    // Assume "index.html" exists in LittleFS
    TEST_ASSERT_TRUE(portal.initialize("TestAP", "12345678", "index.html"));
    TEST_ASSERT_TRUE(portal.startAP());
    TEST_ASSERT_TRUE(portal.stopAP());
    TEST_ASSERT_EQUAL(CaptivePortalError::None, portal.getLastError());
}

void test_get_last_error_string() {
    CaptivePortal portal;
    portal.initialize(nullptr, "12345678", "index.html");
    String errStr = portal.getLastErrorString();
    TEST_ASSERT_EQUAL_STRING(String(static_cast<int>(CaptivePortalError::InvalidSSID)).c_str(), errStr.c_str());
}

void test_open_ap_initialize() {
    CaptivePortal portal;
    // Assume "index.html" exists in LittleFS
    TEST_ASSERT_TRUE(portal.initializeOpen("OpenAP", "index.html"));
    TEST_ASSERT_EQUAL(CaptivePortalError::None, portal.getLastError());
    portal.stopAP();
}

void setup() {
    Serial.begin(115200);
    UNITY_BEGIN();
    RUN_TEST(test_invalid_ssid_null);
    RUN_TEST(test_invalid_ssid_empty);
    RUN_TEST(test_invalid_ssid_too_long);
    RUN_TEST(test_invalid_password_too_short);
    RUN_TEST(test_invalid_password_too_long);
    RUN_TEST(test_file_not_found);
    RUN_TEST(test_already_running);
    RUN_TEST(test_not_initialized);
    RUN_TEST(test_stop_ap_not_running);
    RUN_TEST(test_double_initialize);
    RUN_TEST(test_stop_ap_after_start);
    RUN_TEST(test_get_last_error_string);
    RUN_TEST(test_open_ap_initialize);
    UNITY_END();
}

void loop() {
    // not used
}