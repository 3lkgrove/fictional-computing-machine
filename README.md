# Cheap ESP32 Temperature Monitor-Display
# 🌡️ ESP32-C3 Temperature & Humidity Telemetry Monitor (`v1.0.1`)

[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32--C3-FF6F00?style=for-the-badge&logo=platformio&logoColor=white)](https://platformio.org/)
[![Framework](https://img.shields.io/badge/Framework-Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Sensor](https://img.shields.io/badge/Sensor-Adafruit%20AHT20-38BDF8?style=for-the-badge)](https://www.adafruit.com/product/4566)
[![Status](https://img.shields.io/badge/Status-Active%20Build%20v1.0.1-10B981?style=for-the-badge)](#)

A responsive, single-page web telemetry and Over-The-Air (OTA) monitor built in **PlatformIO** for the **ESP32-C3** paired with an **AHT20** temperature and relative humidity sensor. Designed for headless or benchtop plugged-in monitoring with hardware Task Watchdog recovery, tuned 2.4 GHz radio/power settings, persistent `Preferences` storage, automatic fallback Access Point recovery, POSIX time-zone-aware timestamps, and 24-hour 15-minute CSV data export.

---

## ✨ Key Features

- **🌡️ Live AHT20 Environmental Monitoring**
  - Polls temperature and relative humidity (`Wire` on `SDA = GPIO 5`, `SCL = GPIO 6`).
  - Switch seamlessly between **°F** and **°C** directly from the Web GUI (persisted in NVS `Preferences`).
  - Toggle sensor measurement on/off at runtime without rebooting the controller.
- **📊 24-Hour History (15-Minute Increments), Min/Max & CSV Export**
  - Ring buffer stores up to **96 slots** (24 hours at 15-minute intervals) with local timestamps and uptime fallbacks.
  - Computes **24h Min/Max Temp** and **24h Min/Max Humidity** automatically.
  - Exports a downloadable `esp32c3-24h-history.csv` file containing `Timestamp_Local`, `Unix_Epoch`, `Increment_Offset_Min`, `Uptime_Sec`, `Temp_C`, `Temp_F`, and `Humidity_Pct`.
- **🛡️ Hardware Task Watchdog Timer (`esp_task_wdt`) & Web Diagnostics**
  - Monitors the main loop with a configurable **15-second** timeout (`WDT_TIMEOUT_SEC`) and automatically reboots if execution stalls.
  - Tracks watchdog reset reasons on boot (`ESP_RST_TASK_WDT` / `ESP_RST_INT_WDT`) and surfaces live **Watchdog Status**, **Timeout Window**, and **WDT Reset Count** in the Web GUI and `/api/status`.
  - Includes a **Test WDT Reset** button (`/api/wdt/test`) to verify watchdog failover on demand, and automatically pauses/resumes the watchdog during `/update` OTA firmware uploads.
- **📻 Radio & Power Tuning for Plugged-In ESP32-C3 Operation**
  - **Bluetooth Radio Disabled:** De-initializes the Bluetooth controller at boot (`DISABLE_BLUETOOTH_RADIO`) to prevent 2.4 GHz RF interference and current spikes on shared-antenna ESP32-C3 boards.
  - **8.5 dBm Max WiFi TX Power:** Caps transmit power (`WIFI_POWER_8_5dBm`) to reduce heat and peak power draw while maintaining stable local WiFi performance.
  - **Modem Sleep Disabled:** Keeps WiFi modem sleep off (`WIFI_PS_NONE`) when plugged into USB/wall power for low-latency web dashboard and OTA responsiveness.
- **🕒 NTP Clock & POSIX Time Zone Selector**
  - Syncs clock via `pool.ntp.org` and `time.nist.gov` with automatic Daylight Saving Time support via POSIX `TZ` strings (defaults to `CST6CDT,M3.2.0,M11.1.0`).
  - Includes common regional presets plus a **Custom POSIX TZ** input saved to `Preferences`.
- **📡 Resilient WiFi STA, mDNS & Fallback AP Mode**
  - Connects in Station (`WIFI_STA`) mode, advertises `http://<hostname>.local` via **mDNS**, and displays live **WiFi Signal (RSSI in dBm)** with signal quality rating.
  - Allows updating **Hostname**, **WiFi SSID**, and **WiFi Password** live from the browser.
  - Automatically spins up a **Fallback Access Point** (`ESP-TEMP-MONITOR-SETUP` at `http://192.168.4.1`) if connection fails at boot or after saving credentials, then tears down the AP once station WiFi connects.
- **💡 Built-In WiFi Status LED Indicator**
  - Illuminates while booting, disconnected, or in Fallback AP mode, and turns **OFF** once station WiFi connects (configurable via `LED_BUILTIN_PIN` and `LED_ACTIVE_HIGH`).
- **🎨 Themes, Live Serial Log Viewer & Browser OTA**
  - Switch between **Ocean Blue**, **Emerald**, **Amber**, and **Violet Slate** themes.
  - Toggle runtime serial debug output, inspect a ring-buffered **Live Serial Log Viewer** in the browser with auto-scroll and clear controls, and push `.bin` firmware updates with a live percentage progress bar via `/update`.

    ![WebGui Interface](Images/upper_web_gui.png)
    ![WebGui Interface](Images/lower_web_gui.png)

---

## 📝 Changelog

### `v1.0.1`
- Added hardware **Task Watchdog Timer** (`15s` default timeout) with automatic feeding across `setup()`, `loop()`, and blocking WiFi connection loops.
- Added **Task Watchdog Timer** status card, boot reset counter, and **Test WDT Reset** button (`/api/wdt/test`) to the Web GUI, with automatic watchdog pause/resume during OTA uploads.
- Disabled the onboard **Bluetooth** radio/controller at boot to eliminate 2.4 GHz coexistence overhead.
- Capped maximum **WiFi TX power** at `8.5 dBm` (`WIFI_POWER_8_5dBm`) to reduce thermal load and current draw on compact ESP32-C3 modules.
- Disabled **WiFi modem sleep** (`WiFi.setSleep(false)`) for plugged-in operation to ensure snappy web UI and OTA response times.

### `v1.0.0`
- Initial release with AHT20 telemetry, 24-hour 15-minute history ring buffer, CSV export, NTP + POSIX time zones, Fallback AP recovery, mDNS, live serial log viewer, themes, and browser OTA updates.

---

## 🛠️ Hardware Required

| Component | Specification / Role | Notes |
| :--- | :--- | :--- |
| **Microcontroller** | ESP32-C3 Development Board (`esp32-c3-devkitm-1`) | RISC-V single-core with native USB Serial/JTAG & WiFi |
| **Temp/Humidity Sensor** | **Adafruit AHT20** (I2C Temperature & Humidity) | `3.3V` logic/power, I2C address `0x38` |
| **Onboard Status LED** | Built-in LED on `GPIO 8` (default) | Override `LED_BUILTIN_PIN` / `LED_ACTIVE_HIGH` per board |
| **Wiring & Power** | USB-C data cable + jumper wires (or custom PCB) | Ensure solid `3.3V` and `GND` rails for clean I2C reads |

---

## 🔌 Wiring Diagram

### Pin Connection Table

| AHT20 Sensor Pin | ESP32-C3 Pin | Signal / Function |
| :--- | :--- | :--- |
| `VIN` / `3V3` | `3V3` | `3.3V` regulated supply rail |
| `GND` | `GND` | Common ground reference |
| `SDA` | `GPIO 5` | I2C serial data (`I2C_SDA_PIN = 5`) |
| `SCL` | `GPIO 6` | I2C serial clock (`I2C_SCL_PIN = 6`) |

### Rendered Circuit Diagram

```text
+-----------------------------------+          +---------------------------+
|         ESP32-C3 DevKitM-1        |          |      Adafruit AHT20       |
|                                   |          |                           |
|                         3V3 (3.3V)|--------->| VIN / 3V3                 |
|                                   |          |                           |
|                        GPIO 5 (SDA)|<========>| SDA (I2C Data)           |
|                                   |          |                           |
|                        GPIO 6 (SCL)|--------->| SCL (I2C Clock)          |
|                                   |          |                           |
|                         GND       |<-------->| GND                       |
|                                   |          |                           |
|  [Onboard LED: GPIO 8]            |          |                           |
|   - ON: Boot / Disconnected / AP  |          |                           |
|   - OFF: WiFi Connected           |          |                           |
|                                   |          |                           |
|  [USB-C Port] ------------------->|          |                           |
+-----------------------------------+          +---------------------------+