/*
   fictional-computing machine
   Description:
   See Github Repository
   Notes:
   Contact Info:
   email - 3lkgrove.pm.me
   github - https://github.com/3lkgrove
   Changelog:
   1.0.0 - new code
   1.0.1 - add task watchdog timer, disable BT, cap WiFi TX at 8.5 dBm, disable modem sleep when plugged in
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <Update.h>
#include <WebServer.h>
#include <time.h>
#include <esp_system.h>
#include <esp_task_wdt.h>
#if defined(CONFIG_BT_ENABLED) || defined(SOC_BT_SUPPORTED)
#include <esp_bt.h>
#endif

// Firmware & Project Metadata Macros
#define VER "1.0.1"
#define VER_BUILD "11222026"
#define EMAIL "3lkgrove@pm.me"
#define FIRMWARELINK "https://github.com/3lkgrove/fictional-computing-machine"

// Task Watchdog Timer Configuration
#ifndef WDT_TIMEOUT_SEC
#define WDT_TIMEOUT_SEC 15
#endif

// Radio & Power Management Defaults (tuned for plugged-in ESP32-C3 operation)
#ifndef DISABLE_BLUETOOTH_RADIO
#define DISABLE_BLUETOOTH_RADIO 1
#endif
#ifndef WIFI_MAX_TX_POWER
#define WIFI_MAX_TX_POWER WIFI_POWER_8_5dBm
#endif
#ifndef WIFI_DISABLE_MODEM_SLEEP_WHEN_PLUGGED_IN
#define WIFI_DISABLE_MODEM_SLEEP_WHEN_PLUGGED_IN 1
#endif

// WiFi & Network Defaults
#define WIFI_SSID "CHANGE-ME"
#define WIFI_PASSWORD "CHANGE-ME"
#define DEVICE_HOSTNAME "CHANGE-ME"

// Fallback WiFi Access Point Configuration (used if STA connection fails)
#define FALLBACK_AP_SSID "ESP-TEMP-MONITOR-SETUP"
#define FALLBACK_AP_PASSWORD ""        // Empty = open setup AP (or set >= 8 chars for WPA2)
#define FALLBACK_AP_IP "192.168.4.1"

// NTP & Time Zone Defaults (America/Chicago POSIX rule with US DST)
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.nist.gov"
#define DEFAULT_TIMEZONE "CST6CDT,M3.2.0,M11.1.0"

// Built-in WiFi Status LED Configuration
// ON while booting, disconnected, or in Fallback AP mode; OFF once STA WiFi is connected
#ifndef LED_BUILTIN_PIN
#define LED_BUILTIN_PIN 8
#endif
#ifndef LED_ACTIVE_HIGH
#define LED_ACTIVE_HIGH 0
#endif

// AHT20 I2C Pins & 24h / 15-min History Configuration (96 intervals = 24 hours)
#define I2C_SDA_PIN 5
#define I2C_SCL_PIN 6
#define HISTORY_INTERVAL_MS 900000UL   // 15 minutes
#define HISTORY_MAX_SLOTS 96           // 24 hours * 4 samples/hour
#define SENSOR_POLL_MS 5000UL          // Live readout poll interval

// Compile-time default for serial debug (can be overridden in platformio.ini)
#ifndef APP_DEBUG_SERIAL
#define APP_DEBUG_SERIAL 1
#endif

// Ring buffer sizing for browser Serial Log Viewer
#define LOG_BUFFER_LINES 120
#define LOG_LINE_MAX_LEN 192

struct LogEntry {
    uint32_t seq = 0;
    char text[LOG_LINE_MAX_LEN];
};

struct HistorySample {
    bool valid = false;
    float tempC = NAN;
    float humidity = NAN;
    unsigned long elapsedSec = 0;
    time_t timestamp = 0;
};

// Runtime & Persisted State
struct DeviceState {
    String wifiSsid = WIFI_SSID;
    String wifiPassword = WIFI_PASSWORD;
    String hostname = DEVICE_HOSTNAME;
    String timeZone = DEFAULT_TIMEZONE;
    String ipAddress = "0.0.0.0";
    String apIpAddress = FALLBACK_AP_IP;
    bool apModeActive = false;
    bool timeSynced = false;
    time_t currentEpoch = 0;
    String theme = "ocean";       // "ocean", "emerald", "amber", "slate"
    bool serialDebug = (APP_DEBUG_SERIAL != 0);
    bool sensorEnabled = true;
    bool useFahrenheit = true;
    bool sensorReady = false;
    bool wdtEnabled = false;
    uint32_t wdtTimeoutSec = WDT_TIMEOUT_SEC;
    uint32_t wdtResetCount = 0;
    float tempC = NAN;
    float humidity = NAN;
    unsigned long lastReadMs = 0;
    unsigned long uptimeSec = 0;
};

extern DeviceState gState;
extern Preferences gPrefs;
extern WebServer server;

// Radio & power management helpers
void disableBluetoothRadio();
void applyWifiRadioPowerConfig();

// Task Watchdog Timer helpers
void initWatchdog(uint32_t timeoutSec = WDT_TIMEOUT_SEC);
void feedWatchdog();
void setWatchdogEnabled(bool enabled);
void triggerWatchdogTest();

// Built-in LED, WiFi STA/Fallback AP, Time/TZ, and mDNS helpers
void initWifiLed();
void setWifiLedConnected(bool connected);
void startFallbackAp();
void stopFallbackAp();
void initTimeAndTz();
void applyTimeZone(const String &newTz);
void updateClockState();
void updateWifiStatusIndicator();
void applyHostname(const String &newHostname);
void applyWifiCredentials(const String &newSsid, const String &newPassword);

// Sensor & 24h / 15-min history helpers
void initAht20Sensor();
void setAht20Enabled(bool enabled);
void updateSensorAndHistory();
uint16_t getHistoryCount();
bool getHistorySlotNewestFirst(uint16_t idxFromNewest, HistorySample &out);

// Log ring buffer functions
void appendLog(const char *msg, bool newline);
void appendLogf(const char *fmt, ...);
void clearDebugLogs();
uint32_t getLogCount();
bool getLogEntry(size_t index, LogEntry &out);

// Debug logging macros honoring both compile-time and runtime debug toggle
#define DBG_PRINT(msg) do { \
    if (gState.serialDebug) { \
        Serial.print(msg); \
        appendLog(String(msg).c_str(), false); \
    } \
} while (0)

#define DBG_PRINTLN(msg) do { \
    if (gState.serialDebug) { \
        Serial.println(msg); \
        appendLog(String(msg).c_str(), true); \
    } \
} while (0)

#define DBG_PRINTF(fmt, ...) do { \
    if (gState.serialDebug) { \
        Serial.printf(fmt, ##__VA_ARGS__); \
        appendLogf(fmt, ##__VA_ARGS__); \
    } \
} while (0)

void loadPreferences();
void savePreferences();

// Web server lifecycle & request handling
void initWebServer();
void handleWebClient();

#endif // CONFIG_H