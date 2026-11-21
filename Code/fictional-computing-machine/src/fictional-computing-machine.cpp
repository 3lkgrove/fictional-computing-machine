#include "config.h"
#include <stdarg.h>
#include <freertos/semphr.h>

DeviceState gState;
Preferences gPrefs;

static Adafruit_AHTX0 gAht20;
static HistorySample gHistory[HISTORY_MAX_SLOTS];
static uint16_t gHistoryHead = 0;
static uint16_t gHistoryCount = 0;
static unsigned long gLastBucketMs = 0;
static bool gFirstBucketRecorded = false;

static LogEntry gLogRing[LOG_BUFFER_LINES];
static size_t gLogHead = 0;
static size_t gLogCount = 0;
static uint32_t gLogSeq = 1;
static char gPartialLine[LOG_LINE_MAX_LEN] = {0};
static size_t gPartialLen = 0;
static SemaphoreHandle_t gLogMutex = nullptr;

static bool gLastWifiConnected = false;
static bool gMdnsStarted = false;
static bool gNtpConfigured = false;

static String sanitizeHostname(const String &raw) {
    String lower = raw;
    lower.trim();
    lower.toLowerCase();
    if (lower.endsWith(".local")) {
        lower.remove(lower.length() - 6);
    }

    String clean;
    clean.reserve(lower.length());
    for (size_t i = 0; i < lower.length() && clean.length() < 32; i++) {
        char c = lower[i];
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-') {
            clean += c;
        }
    }
    while (clean.startsWith("-")) clean.remove(0, 1);
    while (clean.endsWith("-")) clean.remove(clean.length() - 1);
    return clean;
}

static void startOrRestartMdns() {
    if (WiFi.status() != WL_CONNECTED) return;
    MDNS.end();
    gMdnsStarted = false;
    if (MDNS.begin(gState.hostname.c_str())) {
        MDNS.addService("http", "tcp", 80);
        gMdnsStarted = true;
        DBG_PRINTF("[mDNS] Responder active at http://%s.local\n", gState.hostname.c_str());
    } else {
        DBG_PRINTLN("[mDNS] Failed to start mDNS responder");
    }
}

void initTimeAndTz() {
    setenv("TZ", gState.timeZone.c_str(), 1);
    tzset();
    configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2);
    gNtpConfigured = true;
    DBG_PRINTF("[TIME] Configured TZ='%s' with NTP servers %s, %s\n",
               gState.timeZone.c_str(), NTP_SERVER_1, NTP_SERVER_2);
}

void applyTimeZone(const String &newTz) {
    String trimmed = newTz;
    trimmed.trim();
    if (trimmed.length() == 0) return;

    gState.timeZone = trimmed;
    setenv("TZ", gState.timeZone.c_str(), 1);
    tzset();
    savePreferences();
    updateClockState();
    DBG_PRINTF("[TIME] Time zone updated to '%s'\n", gState.timeZone.c_str());
}

void updateClockState() {
    if (WiFi.status() == WL_CONNECTED && !gNtpConfigured) {
        initTimeAndTz();
    }

    time_t nowEpoch = time(nullptr);
    if (nowEpoch >= 1704067200) {
        if (!gState.timeSynced) {
            DBG_PRINTF("[TIME] NTP time synchronized (epoch=%ld)\n", (long)nowEpoch);
        }
        gState.timeSynced = true;
        gState.currentEpoch = nowEpoch;
    } else {
        gState.timeSynced = false;
        gState.currentEpoch = 0;
    }
}

void startFallbackAp() {
    if (gState.apModeActive) return;

    WiFi.mode(WIFI_AP_STA);
    IPAddress apIp(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(apIp, apIp, subnet);

    bool started = false;
    if (strlen(FALLBACK_AP_PASSWORD) >= 8) {
        started = WiFi.softAP(FALLBACK_AP_SSID, FALLBACK_AP_PASSWORD);
    } else {
        started = WiFi.softAP(FALLBACK_AP_SSID);
    }

    if (started) {
        gState.apModeActive = true;
        gState.apIpAddress = WiFi.softAPIP().toString();
        setWifiLedConnected(false);
        DBG_PRINTF("[AP] Fallback AP active! Connect to SSID '%s' -> http://%s\n",
                   FALLBACK_AP_SSID, gState.apIpAddress.c_str());
    } else {
        DBG_PRINTLN("[AP] Failed to start Fallback Access Point");
    }
}

void stopFallbackAp() {
    if (!gState.apModeActive) return;
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    gState.apModeActive = false;
    DBG_PRINTLN("[AP] Fallback AP stopped (STA WiFi connected)");
}

void applyHostname(const String &newHostname) {
    String clean = sanitizeHostname(newHostname);
    if (clean.length() == 0) {
        return;
    }
    gState.hostname = clean;
    WiFi.setHostname(gState.hostname.c_str());
    savePreferences();
    DBG_PRINTF("[NET] Hostname updated to %s (%s.local)\n",
               gState.hostname.c_str(), gState.hostname.c_str());
    startOrRestartMdns();
}

void applyWifiCredentials(const String &newSsid, const String &newPassword) {
    String trimmedSsid = newSsid;
    trimmedSsid.trim();
    if (trimmedSsid.length() == 0) {
        return;
    }

    gState.wifiSsid = trimmedSsid;
    if (newPassword.length() > 0) {
        gState.wifiPassword = newPassword;
    }
    savePreferences();

    setWifiLedConnected(false);
    gLastWifiConnected = false;
    gMdnsStarted = false;
    gState.ipAddress = "0.0.0.0";

    DBG_PRINTF("[WIFI] Applying new SSID '%s' and connecting...\n", gState.wifiSsid.c_str());
    WiFi.disconnect(false);
    delay(150);
    WiFi.mode(gState.apModeActive ? WIFI_AP_STA : WIFI_STA);
    WiFi.setHostname(gState.hostname.c_str());
    WiFi.begin(gState.wifiSsid.c_str(), gState.wifiPassword.c_str());

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt) < 15000) {
        delay(500);
        DBG_PRINT(".");
    }
    DBG_PRINTLN("");

    if (WiFi.status() == WL_CONNECTED) {
        stopFallbackAp();
        initTimeAndTz();
        updateWifiStatusIndicator();
    } else {
        DBG_PRINTF("[WIFI] Failed to connect to '%s' — enabling Fallback AP mode\n",
                   gState.wifiSsid.c_str());
        startFallbackAp();
        updateWifiStatusIndicator();
    }
}

static void writeLedState(bool ledOn) {
    uint8_t pinLevel;
    if (LED_ACTIVE_HIGH != 0) {
        pinLevel = ledOn ? HIGH : LOW;
    } else {
        pinLevel = ledOn ? LOW : HIGH;
    }
    digitalWrite(LED_BUILTIN_PIN, pinLevel);
}

void initWifiLed() {
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    writeLedState(true);
    gLastWifiConnected = false;
}

void setWifiLedConnected(bool connected) {
    writeLedState(!connected);
}

void updateWifiStatusIndicator() {
    bool connected = (WiFi.status() == WL_CONNECTED);

    if (connected) {
        if (gState.apModeActive) {
            stopFallbackAp();
        }
        setWifiLedConnected(true);
        gState.ipAddress = WiFi.localIP().toString();

        if (!gLastWifiConnected) {
            DBG_PRINTF("[WIFI] Connected! IP: %s (Built-in LED OFF)\n", gState.ipAddress.c_str());
            if (!gMdnsStarted) {
                startOrRestartMdns();
            }
            initTimeAndTz();
        }
    } else {
        setWifiLedConnected(false);
        gState.ipAddress = "0.0.0.0";
        if (gLastWifiConnected) {
            DBG_PRINTLN("[WIFI] Disconnected (Built-in LED ON)");
            startFallbackAp();
        }
    }

    gLastWifiConnected = connected;
}

static void lockLogs() {
    if (gLogMutex == nullptr) {
        gLogMutex = xSemaphoreCreateMutex();
    }
    if (gLogMutex != nullptr) {
        xSemaphoreTake(gLogMutex, portMAX_DELAY);
    }
}

static void unlockLogs() {
    if (gLogMutex != nullptr) {
        xSemaphoreGive(gLogMutex);
    }
}

static void pushCompleteLineLocked(const char *line) {
    snprintf(gLogRing[gLogHead].text, LOG_LINE_MAX_LEN, "%s", line);
    gLogRing[gLogHead].seq = gLogSeq++;
    gLogHead = (gLogHead + 1) % LOG_BUFFER_LINES;
    if (gLogCount < LOG_BUFFER_LINES) {
        gLogCount++;
    }
}

void appendLog(const char *msg, bool newline) {
    if (msg == nullptr) return;
    lockLogs();

    for (size_t i = 0; msg[i] != '\0'; i++) {
        char c = msg[i];
        if (c == '\r') continue;
        if (c == '\n') {
            gPartialLine[gPartialLen] = '\0';
            pushCompleteLineLocked(gPartialLine);
            gPartialLen = 0;
            gPartialLine[0] = '\0';
        } else if (gPartialLen < (LOG_LINE_MAX_LEN - 1)) {
            gPartialLine[gPartialLen++] = c;
            gPartialLine[gPartialLen] = '\0';
        }
    }

    if (newline) {
        gPartialLine[gPartialLen] = '\0';
        pushCompleteLineLocked(gPartialLine);
        gPartialLen = 0;
        gPartialLine[0] = '\0';
    }

    unlockLogs();
}

void appendLogf(const char *fmt, ...) {
    char buf[LOG_LINE_MAX_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    appendLog(buf, false);
}

void clearDebugLogs() {
    lockLogs();
    gLogHead = 0;
    gLogCount = 0;
    gPartialLen = 0;
    gPartialLine[0] = '\0';
    unlockLogs();
}

uint32_t getLogCount() {
    lockLogs();
    uint32_t c = (uint32_t)gLogCount;
    unlockLogs();
    return c;
}

bool getLogEntry(size_t index, LogEntry &out) {
    lockLogs();
    if (index >= gLogCount) {
        unlockLogs();
        return false;
    }
    size_t oldest = (gLogHead + LOG_BUFFER_LINES - gLogCount) % LOG_BUFFER_LINES;
    size_t slot = (oldest + index) % LOG_BUFFER_LINES;
    out = gLogRing[slot];
    unlockLogs();
    return true;
}

static void recordHistoryPoint(float tempC, float humidity) {
    updateClockState();
    HistorySample sample;
    sample.valid = (!isnan(tempC) && !isnan(humidity));
    sample.tempC = tempC;
    sample.humidity = humidity;
    sample.elapsedSec = millis() / 1000UL;
    sample.timestamp = gState.timeSynced ? gState.currentEpoch : 0;

    gHistory[gHistoryHead] = sample;
    gHistoryHead = (gHistoryHead + 1) % HISTORY_MAX_SLOTS;
    if (gHistoryCount < HISTORY_MAX_SLOTS) {
        gHistoryCount++;
    }
}

static bool pollAht20Once() {
    if (!gState.sensorReady) return false;
    sensors_event_t humidityEvent, tempEvent;
    if (!gAht20.getEvent(&humidityEvent, &tempEvent)) {
        DBG_PRINTLN("[AHT20] Failed to read sensor event");
        return false;
    }
    gState.tempC = tempEvent.temperature;
    gState.humidity = humidityEvent.relative_humidity;
    gState.lastReadMs = millis();
    return true;
}

void initAht20Sensor() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if (!gState.sensorEnabled) {
        DBG_PRINTLN("[AHT20] Sensor polling is disabled in settings");
        gState.sensorReady = false;
        return;
    }
    if (gAht20.begin(&Wire)) {
        gState.sensorReady = true;
        DBG_PRINTF("[AHT20] Initialized on SDA=%d, SCL=%d\n", I2C_SDA_PIN, I2C_SCL_PIN);
        if (pollAht20Once()) {
            recordHistoryPoint(gState.tempC, gState.humidity);
            gLastBucketMs = millis();
            gFirstBucketRecorded = true;
        }
    } else {
        gState.sensorReady = false;
        DBG_PRINTF("[AHT20] Sensor not found on SDA=%d, SCL=%d\n", I2C_SDA_PIN, I2C_SCL_PIN);
    }
}

void setAht20Enabled(bool enabled) {
    gState.sensorEnabled = enabled;
    if (!enabled) {
        DBG_PRINTLN("[AHT20] Measurement turned OFF (device continues running)");
        return;
    }
    DBG_PRINTLN("[AHT20] Measurement turned ON");
    if (!gState.sensorReady) {
        if (gAht20.begin(&Wire)) {
            gState.sensorReady = true;
            DBG_PRINTLN("[AHT20] Re-initialized successfully");
        } else {
            DBG_PRINTLN("[AHT20] Re-initialization failed; check wiring");
            return;
        }
    }
    if (pollAht20Once()) {
        if (!gFirstBucketRecorded) {
            recordHistoryPoint(gState.tempC, gState.humidity);
            gLastBucketMs = millis();
            gFirstBucketRecorded = true;
        }
    }
}

void updateSensorAndHistory() {
    if (!gState.sensorEnabled) {
        return;
    }

    if (!gState.sensorReady) {
        static unsigned long lastRetryMs = 0;
        if (millis() - lastRetryMs >= 10000UL) {
            lastRetryMs = millis();
            if (gAht20.begin(&Wire)) {
                gState.sensorReady = true;
                DBG_PRINTLN("[AHT20] Recovered sensor connection");
            }
        }
        return;
    }

    unsigned long now = millis();
    if (now - gState.lastReadMs >= SENSOR_POLL_MS) {
        if (pollAht20Once()) {
            DBG_PRINTF("[AHT20] Temp: %.2f C (%.2f F), Humidity: %.1f%%\n",
                       gState.tempC, (gState.tempC * 9.0f / 5.0f) + 32.0f, gState.humidity);
        }
    }

    if (!gFirstBucketRecorded || (now - gLastBucketMs >= HISTORY_INTERVAL_MS)) {
        if (!isnan(gState.tempC) && !isnan(gState.humidity)) {
            recordHistoryPoint(gState.tempC, gState.humidity);
            gLastBucketMs = now;
            gFirstBucketRecorded = true;
            DBG_PRINTF("[HISTORY] Recorded 15m sample #%u: %.2f C, %.1f%%\n",
                       gHistoryCount, gState.tempC, gState.humidity);
        }
    }
}

uint16_t getHistoryCount() {
    return gHistoryCount;
}

bool getHistorySlotNewestFirst(uint16_t idxFromNewest, HistorySample &out) {
    if (idxFromNewest >= gHistoryCount) return false;
    int slot = (int)gHistoryHead - 1 - (int)idxFromNewest;
    while (slot < 0) slot += HISTORY_MAX_SLOTS;
    out = gHistory[slot % HISTORY_MAX_SLOTS];
    return true;
}

void loadPreferences() {
    gPrefs.begin("esp32c3cfg", false);
    gState.wifiSsid = gPrefs.getString("wifi_ssid", WIFI_SSID);
    gState.wifiPassword = gPrefs.getString("wifi_pass", WIFI_PASSWORD);
    gState.hostname = gPrefs.getString("hostname", DEVICE_HOSTNAME);
    gState.timeZone = gPrefs.getString("timezone", DEFAULT_TIMEZONE);
    gState.theme = gPrefs.getString("theme", "ocean");
    gState.serialDebug = gPrefs.getBool("debug", (APP_DEBUG_SERIAL != 0));
    gState.sensorEnabled = gPrefs.getBool("sensor_en", true);
    gState.useFahrenheit = gPrefs.getBool("use_f", true);
}

void savePreferences() {
    gPrefs.putString("wifi_ssid", gState.wifiSsid);
    gPrefs.putString("wifi_pass", gState.wifiPassword);
    gPrefs.putString("hostname", gState.hostname);
    gPrefs.putString("timezone", gState.timeZone);
    gPrefs.putString("theme", gState.theme);
    gPrefs.putBool("debug", gState.serialDebug);
    gPrefs.putBool("sensor_en", gState.sensorEnabled);
    gPrefs.putBool("use_f", gState.useFahrenheit);
}

void setup() {
    initWifiLed();

    Serial.begin(115200);
    delay(500);

    loadPreferences();
    setenv("TZ", gState.timeZone.c_str(), 1);
    tzset();

    Serial.println("========================================");
    Serial.printf("VER:          %s\n", VER);
    Serial.printf("VER_BUILD:    %s\n", VER_BUILD);
    Serial.printf("EMAIL:        %s\n", EMAIL);
    Serial.printf("FIRMWARELINK: %s\n", FIRMWARELINK);
    Serial.println("========================================");

    DBG_PRINTF("[BOOT] Compile-time APP_DEBUG_SERIAL=%d, Runtime serialDebug=%d\n",
               APP_DEBUG_SERIAL, gState.serialDebug ? 1 : 0);

    initAht20Sensor();

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(gState.hostname.c_str());
    WiFi.begin(gState.wifiSsid.c_str(), gState.wifiPassword.c_str());

    DBG_PRINTF("[WIFI] Connecting to %s", gState.wifiSsid.c_str());
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt) < 20000) {
        delay(500);
        DBG_PRINT(".");
    }
    DBG_PRINTLN("");

    if (WiFi.status() == WL_CONNECTED) {
        initTimeAndTz();
    } else {
        DBG_PRINTLN("[WIFI] Initial connection timed out — starting Fallback AP mode");
        startFallbackAp();
    }

    updateWifiStatusIndicator();
    updateClockState();

    initWebServer();
}

void loop() {
    updateWifiStatusIndicator();
    updateClockState();
    handleWebClient();
    updateSensorAndHistory();
    gState.uptimeSec = millis() / 1000UL;
}
