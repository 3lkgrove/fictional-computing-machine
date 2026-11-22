#include "config.h"

WebServer server(80);

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0" />
<title>ESP32-C3 Controller &amp; OTA</title>
<style>
  :root {
    --bg: #0b1120;
    --card: #111827;
    --card-border: #1f2937;
    --text: #f3f4f6;
    --muted: #9ca3af;
    --accent: #38bdf8;
    --accent-soft: rgba(56, 189, 248, 0.15);
    --bar-bg: #1e293b;
  }
  body[data-theme="emerald"] {
    --accent: #10b981;
    --accent-soft: rgba(16, 185, 129, 0.15);
  }
  body[data-theme="amber"] {
    --accent: #f59e0b;
    --accent-soft: rgba(245, 158, 11, 0.15);
  }
  body[data-theme="slate"] {
    --accent: #a78bfa;
    --accent-soft: rgba(167, 139, 250, 0.15);
  }
- { box-sizing: border-box; font-family: system-ui, -apple-system, Segoe UI, Roboto, sans-serif; }
  body {
    margin: 0;
    background: var(--bg);
    color: var(--text);
    padding: 20px;
  }
  .container {
    max-width: 880px;
    margin: 0 auto;
    display: grid;
    gap: 16px;
  }
  .card {
    background: var(--card);
    border: 1px solid var(--card-border);
    border-radius: 12px;
    padding: 18px 20px;
  }
  .header-top {
    display: flex;
    flex-wrap: wrap;
    justify-content: space-between;
    align-items: center;
    gap: 12px;
  }
  h1 { margin: 0; font-size: 1.35rem; color: var(--accent); }
  h2 { margin: 0 0 12px 0; font-size: 1.05rem; }
  .meta-row {
    display: flex;
    flex-wrap: wrap;
    gap: 10px 18px;
    margin-top: 10px;
    font-size: 0.9rem;
    color: var(--muted);
  }
  .badge {
    background: var(--accent-soft);
    color: var(--accent);
    padding: 3px 9px;
    border-radius: 999px;
    font-weight: 600;
    font-size: 0.82rem;
  }
  .badge.warn {
    background: rgba(245, 158, 11, 0.18);
    color: #fbbf24;
  }
  .ap-banner {
    display: none;
    margin-top: 12px;
    padding: 10px 12px;
    border-radius: 8px;
    background: rgba(245, 158, 11, 0.15);
    border: 1px solid rgba(245, 158, 11, 0.4);
    color: #fde68a;
    font-size: 0.88rem;
  }
  .grid-2 {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
    gap: 14px;
  }
  .grid-3 {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(190px, 1fr));
    gap: 14px;
  }
  .grid-4 {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(175px, 1fr));
    gap: 12px;
  }
  .stat-box {
    background: var(--bar-bg);
    border-radius: 8px;
    padding: 12px;
  }
  .stat-label { font-size: 0.78rem; color: var(--muted); text-transform: uppercase; }
  .stat-val { font-size: 1.15rem; font-weight: 700; margin-top: 4px; word-break: break-all; }
  .stat-sub { font-size: 0.8rem; color: var(--muted); margin-top: 4px; }
  .progress-wrap {
    width: 100%;
    height: 18px;
    background: var(--bar-bg);
    border-radius: 999px;
    overflow: hidden;
    margin-top: 8px;
  }
  .progress-fill {
    height: 100%;
    width: 0%;
    background: var(--accent);
    transition: width 0.2s ease;
  }
  .controls-row {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 12px;
  }
  select, button, input[type="file"], input[type="text"], input[type="password"] {
    background: var(--bar-bg);
    color: var(--text);
    border: 1px solid var(--card-border);
    border-radius: 8px;
    padding: 8px 12px;
    font-size: 0.9rem;
  }
  button {
    background: var(--accent);
    color: #09101d;
    font-weight: 700;
    border: none;
    cursor: pointer;
  }
  button.secondary {
    background: var(--bar-bg);
    color: var(--text);
    border: 1px solid var(--card-border);
    font-weight: 600;
  }
  button.danger {
    background: rgba(239, 68, 68, 0.18);
    color: #fca5a5;
    border: 1px solid rgba(239, 68, 68, 0.45);
    font-weight: 700;
  }
  button:disabled { opacity: 0.5; cursor: not-allowed; }
  a { color: var(--accent); text-decoration: none; }
  a:hover { text-decoration: underline; }
  #otaStatus, #hostStatus, #wifiStatus, #tzStatus, #wdtStatus {
    margin-top: 10px;
    font-size: 0.9rem;
    color: var(--muted);
  }
  #logBox {
    background: #060913;
    border: 1px solid var(--card-border);
    border-radius: 8px;
    padding: 12px;
    height: 220px;
    overflow-y: auto;
    font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
    font-size: 0.82rem;
    line-height: 1.45;
    white-space: pre-wrap;
    word-break: break-word;
    margin-top: 10px;
    color: #e2e8f0;
  }
  .table-wrap {
    max-height: 300px;
    overflow-y: auto;
    border: 1px solid var(--card-border);
    border-radius: 8px;
    margin-top: 10px;
  }
  table {
    width: 100%;
    border-collapse: collapse;
    font-size: 0.88rem;
  }
  th, td {
    padding: 8px 12px;
    text-align: left;
    border-bottom: 1px solid var(--card-border);
  }
  th {
    position: sticky;
    top: 0;
    background: var(--bar-bg);
    color: var(--accent);
    font-weight: 700;
  }
</style>
</head>
<body data-theme="ocean">
<div class="container">
  <div class="card">
    <div class="header-top">
      <div>
        <h1>ESP32-C3 Control &amp; OTA</h1>
        <div class="meta-row">
          <span>Version: <strong id="verText">-</strong></span>
          <span>Build: <strong id="buildText">-</strong></span>
          <span>Current Time: <strong id="clockText">Syncing...</strong></span>
        </div>
      </div>
      <span class="badge" id="mdnsBadge">mDNS Active</span>
    </div>
    <div class="meta-row">
      <span>Contact: <strong id="emailText">-</strong></span>
      <span>Firmware Link: <a id="fwLink" href="#" target="_blank" rel="noopener">Release Page</a></span>
    </div>
    <div class="ap-banner" id="apBanner">
      Fallback AP Mode is active — connected via hotspot <strong id="apSsidText">ESP32C3-Setup</strong> at <strong id="apIpText">192.168.4.1</strong>. Enter valid WiFi credentials below to reconnect.
    </div>
  </div>

  <div class="card">
    <div class="header-top">
      <h2 style="margin: 0;">AHT20 Temperature &amp; Humidity Monitor</h2>
      <div class="controls-row" style="gap: 10px;">
        <button type="button" class="secondary" id="unitToggleBtn" onclick="toggleUnit()">Unit: °F</button>
        <button type="button" class="secondary" id="csvBtn" onclick="exportHistoryCsv()">Export CSV</button>
        <label style="display: inline-flex; align-items: center; gap: 6px; font-size: 0.9rem; cursor: pointer;">
          <input type="checkbox" id="sensorToggle" checked />
          <span>Sensor Measurement Active</span>
        </label>
      </div>
    </div>

    <div class="grid-2" style="margin-top: 12px;">
      <div class="stat-box">
        <div class="stat-label">Temperature</div>
        <div class="stat-val" id="tempText">--</div>
      </div>
      <div class="stat-box">
        <div class="stat-label">Relative Humidity</div>
        <div class="stat-val" id="humText">--</div>
      </div>
    </div>

    <div class="grid-4" style="margin-top: 12px;">
      <div class="stat-box">
        <div class="stat-label">24h Min Temp</div>
        <div class="stat-val" id="minTempText">--</div>
        <div class="stat-sub">Recorded 24h low</div>
      </div>
      <div class="stat-box">
        <div class="stat-label">24h Max Temp</div>
        <div class="stat-val" id="maxTempText">--</div>
        <div class="stat-sub">Recorded 24h high</div>
      </div>
      <div class="stat-box">
        <div class="stat-label">24h Min Humidity</div>
        <div class="stat-val" id="minHumText">--</div>
        <div class="stat-sub">Recorded 24h low</div>
      </div>
      <div class="stat-box">
        <div class="stat-label">24h Max Humidity</div>
        <div class="stat-val" id="maxHumText">--</div>
        <div class="stat-sub">Recorded 24h high</div>
      </div>
    </div>

    <div style="margin-top: 16px;">
      <div class="header-top">
        <span style="font-size: 0.92rem; font-weight: 600;">24-Hour History (15-Minute Increments)</span>
        <span id="histCountLabel" style="font-size: 0.84rem; color: var(--muted);">0 / 96 slots</span>
      </div>
      <div class="table-wrap">
        <table>
          <thead>
            <tr>
              <th>Increment (Newest First)</th>
              <th>Timestamp</th>
              <th id="tempColHeader">Temperature (°F)</th>
              <th>Humidity (%)</th>
            </tr>
          </thead>
          <tbody id="historyBody">
            <tr><td colspan="4" style="color: var(--muted);">No 15-minute samples recorded yet.</td></tr>
          </tbody>
        </table>
      </div>
    </div>
  </div>

  <div class="card">
    <h2>Network &amp; Memory</h2>
    <div class="grid-3">
      <div class="stat-box">
        <div class="stat-label">Hostname</div>
        <div class="stat-val" id="hostText">-</div>
      </div>
      <div class="stat-box">
        <div class="stat-label">IP Address</div>
        <div class="stat-val" id="ipText">-</div>
      </div>
      <div class="stat-box">
        <div class="stat-label">WiFi Signal (RSSI)</div>
        <div class="stat-val" id="rssiText">--</div>
        <div class="stat-sub" id="rssiQualityText">Not connected</div>
      </div>
    </div>

    <div class="controls-row" style="margin-top: 14px;">
      <label for="hostnameInput">Change Hostname:</label>
      <input type="text" id="hostnameInput" maxlength="32" placeholder="esp32c3-node" />
      <button type="button" id="saveHostBtn" onclick="saveHostname()">Save Hostname</button>
    </div>
    <div id="hostStatus">Uses letters, numbers, and hyphens; accessible via mDNS as <code>&lt;hostname&gt;.local</code>.</div>

    <div class="controls-row" style="margin-top: 14px;">
      <label for="wifiSsidInput">WiFi SSID:</label>
      <input type="text" id="wifiSsidInput" maxlength="63" placeholder="Enter WiFi SSID" />
      <label for="wifiPassInput">Password:</label>
      <input type="password" id="wifiPassInput" maxlength="63" placeholder="Leave blank to keep current" />
      <button type="button" id="saveWifiBtn" onclick="saveWifiCredentials()">Save &amp; Connect WiFi</button>
    </div>
    <div id="wifiStatus">Configured SSID: <strong id="currentSsidText">-</strong></div>

    <div style="margin-top: 14px;">
      <div class="header-top" style="font-size: 0.88rem;">
        <span>System Heap Memory Usage</span>
        <span id="memLabel">0 / 0 bytes (0%)</span>
      </div>
      <div class="progress-wrap">
        <div class="progress-fill" id="memBar"></div>
      </div>
    </div>
  </div>

  <div class="card">
    <h2>Time Zone, Appearance &amp; Serial Debug</h2>
    <div class="controls-row">
      <label for="tzPresetSelect">Time Zone:</label>
      <select id="tzPresetSelect" onchange="handleTzPresetChange()">
        <option value="EST5EDT,M3.2.0,M11.1.0">US Eastern (EST / EDT)</option>
        <option value="CST6CDT,M3.2.0,M11.1.0">US Central (CST / CDT)</option>
        <option value="MST7MDT,M3.2.0,M11.1.0">US Mountain (MST / MDT)</option>
        <option value="MST7">US Mountain No DST (MST)</option>
        <option value="PST8PDT,M3.2.0,M11.1.0">US Pacific (PST / PDT)</option>
        <option value="AKST9AKDT,M3.2.0,M11.1.0">Alaska (AKST / AKDT)</option>
        <option value="HST10">Hawaii (HST)</option>
        <option value="UTC0">UTC (Coordinated Universal Time)</option>
        <option value="GMT0BST,M3.5.0/1,M10.5.0">Europe/London (GMT / BST)</option>
        <option value="CET-1CEST,M3.5.0,M10.5.0/3">Europe/Berlin &amp; Paris (CET / CEST)</option>
        <option value="custom">Custom POSIX TZ...</option>
      </select>
      <input type="text" id="tzCustomInput" maxlength="64" placeholder="POSIX TZ string" style="display: none; min-width: 220px;" />
      <button type="button" id="saveTzBtn" onclick="saveTimeZone()">Save Time Zone</button>
    </div>
    <div id="tzStatus">Active TZ: <strong id="currentTzText">-</strong></div>

    <div class="controls-row" style="margin-top: 14px;">
      <label for="themeSelect">Color Theme:</label>
      <select id="themeSelect">
        <option value="ocean">Ocean Blue</option>
        <option value="emerald">Emerald</option>
        <option value="amber">Amber</option>
        <option value="slate">Violet Slate</option>
      </select>

      <label style="margin-left: 10px; display: inline-flex; align-items: center; gap: 6px; cursor: pointer;">
        <input type="checkbox" id="debugToggle" />
        <span>Runtime Serial Debug</span>
      </label>
    </div>
  </div>

  <div class="card">
    <div class="header-top">
      <h2 style="margin: 0;">Task Watchdog Timer</h2>
      <button type="button" class="danger" id="wdtTestBtn" onclick="triggerWatchdogTest()">Test WDT Reset</button>
    </div>
    <div class="grid-3" style="margin-top: 12px;">
      <div class="stat-box">
        <div class="stat-label">Watchdog Status</div>
        <div class="stat-val" id="wdtStateText">-</div>
      </div>
      <div class="stat-box">
        <div class="stat-label">Timeout Window</div>
        <div class="stat-val" id="wdtTimeoutText">-</div>
      </div>
      <div class="stat-box">
        <div class="stat-label">WDT Reset Count</div>
        <div class="stat-val" id="wdtResetsText">0</div>
      </div>
    </div>
    <div id="wdtStatus">Automatically pauses during OTA firmware uploads and resumes afterward.</div>
  </div>

  <div class="card">
    <div class="header-top">
      <h2 style="margin: 0;">Live Serial Log Viewer</h2>
      <div class="controls-row" style="gap: 10px;">
        <label style="display: inline-flex; align-items: center; gap: 6px; font-size: 0.88rem; cursor: pointer;">
          <input type="checkbox" id="autoScrollToggle" checked />
          <span>Auto-scroll</span>
        </label>
        <button type="button" class="secondary" onclick="clearLogs()">Clear Logs</button>
      </div>
    </div>
    <div id="logBox">Waiting for debug output...</div>
  </div>

  <div class="card">
    <h2>Over-The-Air (OTA) Firmware Update</h2>
    <div class="controls-row">
      <input type="file" id="otaFile" accept=".bin" />
      <button id="otaBtn" onclick="uploadFirmware()">Upload Firmware</button>
    </div>
    <div class="progress-wrap">
      <div class="progress-fill" id="otaBar"></div>
    </div>
    <div id="otaStatus">Idle — select a compiled <code>firmware.bin</code> file.</div>
  </div>
</div>

<script>
let lastLogSeq = 0;
let initializedLogs = false;
let latestStatus = null;
let hostnameDirty = false;
let ssidDirty = false;
let tzDirty = false;

function cToF(c) {
  return (c * 9) / 5 + 32;
}

function formatTemp(tempC, useF) {
  if (tempC === null || tempC === undefined || Number.isNaN(tempC)) return "--";
  const val = useF ? cToF(tempC) : tempC;
  return `${val.toFixed(1)} ${useF ? "°F" : "°C"}`;
}

function formatAge(sec) {
  if (!sec || sec < 60) return `${sec || 0}s`;
  const mins = Math.floor(sec / 60);
  if (mins < 60) return `${mins}m`;
  const hrs = Math.floor(mins / 60);
  const remM = mins % 60;
  return `${hrs}h ${remM}m`;
}

function formatEpochLocal(epochSec) {
  if (!epochSec || Number(epochSec) < 1704067200) return null;
  const d = new Date(Number(epochSec) * 1000);
  return d.toLocaleString([], {
    year: "numeric",
    month: "short",
    day: "2-digit",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit"
  });
}

function formatCsvTimestamp(epochSec, fallbackAgeSec) {
  if (epochSec && Number(epochSec) >= 1704067200) {
    const d = new Date(Number(epochSec) * 1000);
    const pad = (n) => String(n).padStart(2, "0");
    return `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())} ${pad(d.getHours())}:${pad(d.getMinutes())}:${pad(d.getSeconds())}`;
  }
  return `Uptime +${formatAge(fallbackAgeSec)}`;
}

function describeRssi(rssi) {
  if (rssi === null || rssi === undefined || Number.isNaN(rssi) || rssi >= 0) {
    return { val: "--", qual: "Not connected" };
  }
  const n = Number(rssi);
  let qual = "Weak";
  if (n >= -55) qual = "Excellent";
  else if (n >= -67) qual = "Good";
  else if (n >= -75) qual = "Fair";
  return { val: `${n} dBm`, qual };
}

function applyTheme(t) {
  document.body.setAttribute("data-theme", t);
  document.getElementById("themeSelect").value = t;
}

function syncTzUi(tzStr) {
  document.getElementById("currentTzText").textContent = tzStr || "-";
  if (tzDirty || document.activeElement === document.getElementById("tzCustomInput")) {
    return;
  }
  const sel = document.getElementById("tzPresetSelect");
  const customInp = document.getElementById("tzCustomInput");
  const hasOption = Array.from(sel.options).some((o) => o.value === tzStr && o.value !== "custom");
  if (hasOption) {
    sel.value = tzStr;
    customInp.style.display = "none";
  } else {
    sel.value = "custom";
    customInp.value = tzStr || "";
    customInp.style.display = "inline-block";
  }
}

function handleTzPresetChange() {
  tzDirty = true;
  const sel = document.getElementById("tzPresetSelect");
  const customInp = document.getElementById("tzCustomInput");
  if (sel.value === "custom") {
    customInp.style.display = "inline-block";
    if (!customInp.value && latestStatus) {
      customInp.value = latestStatus.timezone || "";
    }
    customInp.focus();
  } else {
    customInp.style.display = "none";
    customInp.value = sel.value;
  }
}

async function saveTimeZone() {
  const sel = document.getElementById("tzPresetSelect");
  const customInp = document.getElementById("tzCustomInput");
  const statusEl = document.getElementById("tzStatus");
  const btnEl = document.getElementById("saveTzBtn");

  const tzValue = (sel.value === "custom") ? customInp.value.trim() : sel.value;
  if (!tzValue) {
    statusEl.textContent = "Please select or enter a valid POSIX time zone string.";
    return;
  }

  btnEl.disabled = true;
  statusEl.textContent = "Saving time zone...";

  try {
    const res = await fetch("/api/settings", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ timezone: tzValue })
    });
    if (res.ok) {
      tzDirty = false;
      await refreshStatus();
      statusEl.innerHTML = `Active TZ: <strong>${tzValue}</strong> — Current Time: <strong>${document.getElementById("clockText").textContent}</strong>`;
    } else {
      statusEl.textContent = "Failed to save time zone.";
    }
  } catch (e) {
    statusEl.textContent = "Error saving time zone.";
  } finally {
    btnEl.disabled = false;
  }
}

async function triggerWatchdogTest() {
  const statusEl = document.getElementById("wdtStatus");
  const btnEl = document.getElementById("wdtTestBtn");
  const sec = (latestStatus && latestStatus.wdt_timeout_sec) ? latestStatus.wdt_timeout_sec : 15;

  if (!confirm(`Trigger a real Task Watchdog reset test? The ESP32-C3 will stall and reboot in ~${sec} seconds.`)) {
    return;
  }

  btnEl.disabled = true;
  statusEl.textContent = `Watchdog test triggered — device will reboot in ~${sec}s...`;

  try {
    await fetch("/api/wdt/test", { method: "POST" });
  } catch (e) {
    // Expected if connection drops as device resets
  }

  setTimeout(() => {
    location.reload();
  }, (sec + 4) * 1000);
}

function computeMinMax(hist) {
  let minT = null, maxT = null, minH = null, maxH = null;
  for (const item of hist) {
    if (!item.valid) continue;
    if (item.temp_c !== null && item.temp_c !== undefined && !Number.isNaN(item.temp_c)) {
      const t = Number(item.temp_c);
      if (minT === null || t < minT) minT = t;
      if (maxT === null || t > maxT) maxT = t;
    }
    if (item.humidity !== null && item.humidity !== undefined && !Number.isNaN(item.humidity)) {
      const h = Number(item.humidity);
      if (minH === null || h < minH) minH = h;
      if (maxH === null || h > maxH) maxH = h;
    }
  }
  return { minT, maxT, minH, maxH };
}

function renderSensorAndHistory(d) {
  const useF = Boolean(d.use_fahrenheit);
  document.getElementById("unitToggleBtn").textContent = useF ? "Unit: °F" : "Unit: °C";
  document.getElementById("tempColHeader").textContent = useF ? "Temperature (°F)" : "Temperature (°C)";
  document.getElementById("sensorToggle").checked = Boolean(d.sensor_enabled);

  if (!d.sensor_enabled) {
    document.getElementById("tempText").textContent = "Disabled";
    document.getElementById("humText").textContent = "Disabled";
  } else if (!d.sensor_ready) {
    document.getElementById("tempText").textContent = "Sensor N/A";
    document.getElementById("humText").textContent = "Sensor N/A";
  } else {
    document.getElementById("tempText").textContent = formatTemp(d.temp_c, useF);
    document.getElementById("humText").textContent =
      (d.humidity !== null && !Number.isNaN(d.humidity)) ? `${Number(d.humidity).toFixed(1)} %` : "--";
  }

  const hist = Array.isArray(d.history) ? d.history : [];
  const mm = computeMinMax(hist);
  document.getElementById("minTempText").textContent = mm.minT !== null ? formatTemp(mm.minT, useF) : "--";
  document.getElementById("maxTempText").textContent = mm.maxT !== null ? formatTemp(mm.maxT, useF) : "--";
  document.getElementById("minHumText").textContent = mm.minH !== null ? `${mm.minH.toFixed(1)} %` : "--";
  document.getElementById("maxHumText").textContent = mm.maxH !== null ? `${mm.maxH.toFixed(1)} %` : "--";

  document.getElementById("histCountLabel").textContent = `${hist.length} / 96 slots`;
  const tbody = document.getElementById("historyBody");

  if (hist.length === 0) {
    tbody.innerHTML = '<tr><td colspan="4" style="color: var(--muted);">No 15-minute samples recorded yet.</td></tr>';
    return;
  }

  let rowsHtml = "";
  for (let i = 0; i < hist.length; i++) {
    const item = hist[i];
    const slotLabel = i === 0 ? "Latest (-0m)" : `-${i * 15} min`;
    const formattedClock = formatEpochLocal(item.timestamp);
    const timeLabel = formattedClock ? formattedClock : `Uptime +${formatAge(item.elapsed_sec)}`;
    const tStr = item.valid ? formatTemp(item.temp_c, useF) : "--";
    const hStr = item.valid && item.humidity !== null ? `${Number(item.humidity).toFixed(1)} %` : "--";
    rowsHtml += `<tr><td>${slotLabel}</td><td>${timeLabel}</td><td>${tStr}</td><td>${hStr}</td></tr>`;
  }
  tbody.innerHTML = rowsHtml;
}

function exportHistoryCsv() {
  const hist = (latestStatus && Array.isArray(latestStatus.history)) ? latestStatus.history : [];
  const lines = ["Timestamp_Local,Unix_Epoch,Increment_Offset_Min,Uptime_Sec,Temp_C,Temp_F,Humidity_Pct"];
  for (let i = 0; i < hist.length; i++) {
    const item = hist[i];
    const epoch = (item.timestamp && Number(item.timestamp) >= 1704067200) ? Number(item.timestamp) : "";
    const tsStr = formatCsvTimestamp(item.timestamp, item.elapsed_sec);
    const offsetMin = -(i * 15);
    const upSec = item.elapsed_sec || 0;
    const tC = (item.valid && item.temp_c !== null && !Number.isNaN(item.temp_c)) ? Number(item.temp_c).toFixed(2) : "";
    const tF = (item.valid && item.temp_c !== null && !Number.isNaN(item.temp_c)) ? cToF(Number(item.temp_c)).toFixed(2) : "";
    const hum = (item.valid && item.humidity !== null && !Number.isNaN(item.humidity)) ? Number(item.humidity).toFixed(1) : "";
    lines.push(`"${tsStr}",${epoch},${offsetMin},${upSec},${tC},${tF},${hum}`);
  }

  const blob = new Blob([lines.join("\n")], { type: "text/csv;charset=utf-8;" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = "esp32c3-24h-history.csv";
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}

async function refreshStatus() {
  try {
    const res = await fetch("/api/status");
    if (!res.ok) return;
    const d = await res.json();
    latestStatus = d;

    document.getElementById("verText").textContent = d.ver;
    document.getElementById("buildText").textContent = d.ver_build;
    document.getElementById("emailText").textContent = d.email;
    const linkEl = document.getElementById("fwLink");
    linkEl.href = d.firmware_link;
    linkEl.textContent = d.firmware_link;

    const localClock = formatEpochLocal(d.epoch);
    document.getElementById("clockText").textContent =
      localClock ? localClock : (d.time_synced ? "Synced" : "Awaiting NTP");

    const badgeEl = document.getElementById("mdnsBadge");
    const apBannerEl = document.getElementById("apBanner");
    if (d.ap_mode) {
      badgeEl.textContent = `Fallback AP: ${d.ap_ssid}`;
      badgeEl.classList.add("warn");
      apBannerEl.style.display = "block";
      document.getElementById("apSsidText").textContent = d.ap_ssid;
      document.getElementById("apIpText").textContent = d.ap_ip;
    } else {
      badgeEl.textContent = "mDNS Active";
      badgeEl.classList.remove("warn");
      apBannerEl.style.display = "none";
    }

    document.getElementById("hostText").textContent = d.hostname + ".local";
    const hostInput = document.getElementById("hostnameInput");
    if (!hostnameDirty && document.activeElement !== hostInput) {
      hostInput.value = d.hostname;
    }

    document.getElementById("currentSsidText").textContent = d.wifi_ssid || "-";
    const ssidInput = document.getElementById("wifiSsidInput");
    if (!ssidDirty && document.activeElement !== ssidInput && d.wifi_ssid) {
      ssidInput.value = d.wifi_ssid;
    }

    document.getElementById("ipText").textContent = d.ap_mode ? `${d.ip} (AP: ${d.ap_ip})` : d.ip;

    const rssiInfo = describeRssi(d.wifi_rssi);
    document.getElementById("rssiText").textContent = rssiInfo.val;
    document.getElementById("rssiQualityText").textContent = rssiInfo.qual;

    document.getElementById("wdtStateText").textContent = d.wdt_enabled ? "Active" : "Paused";
    document.getElementById("wdtTimeoutText").textContent = `${d.wdt_timeout_sec || 15}s`;
    document.getElementById("wdtResetsText").textContent = String(d.wdt_reset_count || 0);

    syncTzUi(d.timezone);
    document.getElementById("debugToggle").checked = d.serial_debug;
    applyTheme(d.theme);

    const used = Math.max(0, d.heap_total - d.heap_free);
    const pct = d.heap_total > 0 ? Math.round((used / d.heap_total) * 100) : 0;
    document.getElementById("memBar").style.width = pct + "%";
    document.getElementById("memLabel").textContent =
      `${used.toLocaleString()} / ${d.heap_total.toLocaleString()} B used (${pct}%)`;

    renderSensorAndHistory(d);
  } catch (e) {
    // Ignore transient network blips during reboot or AP fallback
  }
}

async function saveHostname() {
  const inputEl = document.getElementById("hostnameInput");
  const statusEl = document.getElementById("hostStatus");
  const btnEl = document.getElementById("saveHostBtn");
  const raw = inputEl.value.trim();

  if (!raw) {
    statusEl.textContent = "Please enter a valid hostname first.";
    return;
  }

  btnEl.disabled = true;
  statusEl.textContent = "Saving hostname and restarting mDNS...";

  try {
    const res = await fetch("/api/settings", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ hostname: raw })
    });
    if (res.ok) {
      hostnameDirty = false;
      statusEl.textContent = "Hostname saved! Refreshing status...";
      await refreshStatus();
      statusEl.textContent = `Active at http://${document.getElementById("hostText").textContent}`;
    } else {
      statusEl.textContent = "Failed to save hostname.";
    }
  } catch (e) {
    statusEl.textContent = "Error saving hostname.";
  } finally {
    btnEl.disabled = false;
  }
}

async function saveWifiCredentials() {
  const ssidEl = document.getElementById("wifiSsidInput");
  const passEl = document.getElementById("wifiPassInput");
  const statusEl = document.getElementById("wifiStatus");
  const btnEl = document.getElementById("saveWifiBtn");
  const ssid = ssidEl.value.trim();
  const password = passEl.value;

  if (!ssid) {
    statusEl.textContent = "Please enter a WiFi SSID first.";
    return;
  }

  btnEl.disabled = true;
  statusEl.textContent = `Testing connection to "${ssid}" (will activate Fallback AP if connection fails)...`;

  try {
    const res = await fetch("/api/settings", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ wifi_ssid: ssid, wifi_password: password })
    });
    if (res.ok) {
      ssidDirty = false;
      passEl.value = "";
      setTimeout(async () => {
        await refreshStatus();
        if (latestStatus && latestStatus.ap_mode) {
          statusEl.innerHTML = `Could not connect to <strong>${ssid}</strong>. Fallback AP <strong>${latestStatus.ap_ssid}</strong> is active at <strong>http://${latestStatus.ap_ip}</strong>.`;
        } else {
          statusEl.innerHTML = `Connected to <strong>${ssid}</strong> — IP: <strong>${document.getElementById("ipText").textContent}</strong>`;
        }
      }, 1500);
    } else {
      statusEl.textContent = "Failed to save WiFi credentials.";
    }
  } catch (e) {
    statusEl.textContent = "Network switched — check your WiFi IP or connect to the Fallback AP hotspot.";
  } finally {
    btnEl.disabled = false;
  }
}

async function toggleUnit() {
  if (!latestStatus) return;
  latestStatus.use_fahrenheit = !Boolean(latestStatus.use_fahrenheit);
  renderSensorAndHistory(latestStatus);
  await fetch("/api/settings", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ use_fahrenheit: latestStatus.use_fahrenheit })
  });
}

async function pushSettings() {
  const payload = {
    theme: document.getElementById("themeSelect").value,
    serial_debug: document.getElementById("debugToggle").checked,
    sensor_enabled: document.getElementById("sensorToggle").checked
  };
  applyTheme(payload.theme);
  if (latestStatus) {
    latestStatus.sensor_enabled = payload.sensor_enabled;
    renderSensorAndHistory(latestStatus);
  }
  await fetch("/api/settings", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload)
  });
  refreshStatus();
}

async function refreshLogs() {
  try {
    const res = await fetch(`/api/logs?since=${lastLogSeq}`);
    if (!res.ok) return;
    const d = await res.json();
    const logBox = document.getElementById("logBox");

    if (!initializedLogs) {
      logBox.textContent = "";
      initializedLogs = true;
    }

    if (Array.isArray(d.lines) && d.lines.length > 0) {
      for (const item of d.lines) {
        logBox.textContent += item.text + "\n";
        if (item.seq > lastLogSeq) {
          lastLogSeq = item.seq;
        }
      }
      if (document.getElementById("autoScrollToggle").checked) {
        logBox.scrollTop = logBox.scrollHeight;
      }
    }
  } catch (e) {
    // Ignore transient network blips
  }
}

async function clearLogs() {
  await fetch("/api/logs/clear", { method: "POST" });
  lastLogSeq = 0;
  document.getElementById("logBox").textContent = "";
}

function uploadFirmware() {
  const fileInput = document.getElementById("otaFile");
  const statusEl = document.getElementById("otaStatus");
  const barEl = document.getElementById("otaBar");
  const btnEl = document.getElementById("otaBtn");

  if (!fileInput.files || fileInput.files.length === 0) {
    statusEl.textContent = "Please choose a .bin firmware file first.";
    return;
  }

  const file = fileInput.files[0];
  const formData = new FormData();
  formData.append("update", file, file.name);

  btnEl.disabled = true;
  barEl.style.width = "0%";
  statusEl.textContent = "Uploading: 0%";

  const xhr = new XMLHttpRequest();
  xhr.open("POST", "/update", true);

  xhr.upload.onprogress = function(evt) {
    if (evt.lengthComputable) {
      const pct = Math.round((evt.loaded / evt.total) * 100);
      barEl.style.width = pct + "%";
      statusEl.textContent = `Uploading firmware: ${pct}% (${evt.loaded.toLocaleString()} / ${evt.total.toLocaleString()} bytes)`;
    }
  };

  xhr.onload = function() {
    if (xhr.status === 200) {
      barEl.style.width = "100%";
      statusEl.textContent = "Upload complete (100%)! ESP32-C3 is rebooting...";
      setTimeout(() => location.reload(), 4000);
    } else {
      statusEl.textContent = "OTA failed: " + xhr.responseText;
      btnEl.disabled = false;
    }
  };

  xhr.onerror = function() {
    statusEl.textContent = "OTA upload error occurred.";
    btnEl.disabled = false;
  };

  xhr.send(formData);
}

document.getElementById("hostnameInput").addEventListener("input", () => {
  hostnameDirty = true;
});
document.getElementById("hostnameInput").addEventListener("keydown", (e) => {
  if (e.key === "Enter") saveHostname();
});
document.getElementById("wifiSsidInput").addEventListener("input", () => {
  ssidDirty = true;
});
document.getElementById("tzCustomInput").addEventListener("input", () => {
  tzDirty = true;
});
document.getElementById("tzCustomInput").addEventListener("keydown", (e) => {
  if (e.key === "Enter") saveTimeZone();
});
document.getElementById("themeSelect").addEventListener("change", pushSettings);
document.getElementById("debugToggle").addEventListener("change", pushSettings);
document.getElementById("sensorToggle").addEventListener("change", pushSettings);

refreshStatus();
refreshLogs();
setInterval(refreshStatus, 2500);
setInterval(refreshLogs, 1500);
</script>
</body>
</html>
)rawliteral";

static String escapeJson(const String &in) {
    String out;
    out.reserve(in.length() + 8);
    for (size_t i = 0; i < in.length(); i++) {
        char c = in[i];
        if (c == '\\' || c == '"') out += '\\';
        else if (c == '\n') { out += "\\n"; continue; }
        else if (c == '\r') { out += "\\r"; continue; }
        else if (c == '\t') { out += "\\t"; continue; }
        out += c;
    }
    return out;
}

static String floatToJson(float v) {
    if (isnan(v)) return "null";
    return String(v, 2);
}

static String extractJsonStringField(const String &body, const char *key) {
    String pattern = String("\"") + key + "\"";
    int kPos = body.indexOf(pattern);
    if (kPos < 0) return "";
    int colonPos = body.indexOf(':', kPos + pattern.length());
    if (colonPos < 0) return "";
    int firstQuote = body.indexOf('"', colonPos + 1);
    if (firstQuote < 0) return "";

    String val;
    for (int i = firstQuote + 1; i < (int)body.length(); i++) {
        char c = body[i];
        if (c == '\\' && i + 1 < (int)body.length()) {
            val += body[++i];
        } else if (c == '"') {
            break;
        } else {
            val += c;
        }
    }
    return val;
}

static void handleRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}

static void handleStatus() {
    uint32_t heapTotal = ESP.getHeapSize();
    uint32_t heapFree = ESP.getFreeHeap();
    bool wifiConnected = (WiFi.status() == WL_CONNECTED);
    int rssiDbm = wifiConnected ? WiFi.RSSI() : 0;

    String json = "{";
    json += "\"ver\":\"" + escapeJson(VER) + "\",";
    json += "\"ver_build\":\"" + escapeJson(VER_BUILD) + "\",";
    json += "\"email\":\"" + escapeJson(EMAIL) + "\",";
    json += "\"firmware_link\":\"" + escapeJson(FIRMWARELINK) + "\",";
    json += "\"timezone\":\"" + escapeJson(gState.timeZone) + "\",";
    json += "\"time_synced\":" + String(gState.timeSynced ? "true" : "false") + ",";
    json += "\"epoch\":" + String((long)gState.currentEpoch) + ",";
    json += "\"wifi_ssid\":\"" + escapeJson(gState.wifiSsid) + "\",";
    json += "\"wifi_rssi\":" + (wifiConnected ? String(rssiDbm) : String("null")) + ",";
    json += "\"hostname\":\"" + escapeJson(gState.hostname) + "\",";
    json += "\"ip\":\"" + escapeJson(gState.ipAddress) + "\",";
    json += "\"ap_mode\":" + String(gState.apModeActive ? "true" : "false") + ",";
    json += "\"ap_ssid\":\"" + escapeJson(FALLBACK_AP_SSID) + "\",";
    json += "\"ap_ip\":\"" + escapeJson(gState.apIpAddress) + "\",";
    json += "\"theme\":\"" + escapeJson(gState.theme) + "\",";
    json += "\"serial_debug\":" + String(gState.serialDebug ? "true" : "false") + ",";
    json += "\"sensor_enabled\":" + String(gState.sensorEnabled ? "true" : "false") + ",";
    json += "\"use_fahrenheit\":" + String(gState.useFahrenheit ? "true" : "false") + ",";
    json += "\"sensor_ready\":" + String(gState.sensorReady ? "true" : "false") + ",";
    json += "\"wdt_enabled\":" + String(gState.wdtEnabled ? "true" : "false") + ",";
    json += "\"wdt_timeout_sec\":" + String(gState.wdtTimeoutSec) + ",";
    json += "\"wdt_reset_count\":" + String(gState.wdtResetCount) + ",";
    json += "\"temp_c\":" + floatToJson(gState.tempC) + ",";
    json += "\"humidity\":" + floatToJson(gState.humidity) + ",";
    json += "\"heap_total\":" + String(heapTotal) + ",";
    json += "\"heap_free\":" + String(heapFree) + ",";

    json += "\"history\":[";
    uint16_t hCount = getHistoryCount();
    for (uint16_t i = 0; i < hCount; i++) {
        HistorySample s;
        if (!getHistorySlotNewestFirst(i, s)) continue;
        if (i > 0) json += ",";
        json += "{";
        json += "\"valid\":" + String(s.valid ? "true" : "false") + ",";
        json += "\"temp_c\":" + floatToJson(s.tempC) + ",";
        json += "\"humidity\":" + floatToJson(s.humidity) + ",";
        json += "\"elapsed_sec\":" + String(s.elapsedSec) + ",";
        json += "\"timestamp\":" + String((long)s.timestamp);
        json += "}";
    }
    json += "]";
    json += "}";

    server.send(200, "application/json", json);
}

static void handleLogs() {
    uint32_t sinceSeq = 0;
    if (server.hasArg("since")) {
        sinceSeq = (uint32_t)server.arg("since").toInt();
    }

    uint32_t count = getLogCount();
    String json = "{\"lines\":[";
    bool first = true;

    for (uint32_t i = 0; i < count; i++) {
        LogEntry entry;
        if (!getLogEntry(i, entry)) continue;
        if (entry.seq <= sinceSeq) continue;

        if (!first) json += ",";
        first = false;
        json += "{\"seq\":" + String(entry.seq) + ",\"text\":\"" + escapeJson(String(entry.text)) + "\"}";
    }

    json += "]}";
    server.send(200, "application/json", json);
}

static void handleClearLogs() {
    clearDebugLogs();
    server.send(200, "application/json", "{\"ok\":true}");
}

static void handleWdtTest() {
    server.send(200, "application/json", "{\"ok\":true,\"message\":\"Watchdog reset test started\"}");
    server.handleClient();
    triggerWatchdogTest();
}

static void handleSettings() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Missing body\"}");
        return;
    }

    String body = server.arg("plain");

    if (body.indexOf("\"emerald\"") >= 0) gState.theme = "emerald";
    else if (body.indexOf("\"amber\"") >= 0) gState.theme = "amber";
    else if (body.indexOf("\"slate\"") >= 0) gState.theme = "slate";
    else if (body.indexOf("\"ocean\"") >= 0) gState.theme = "ocean";

    if (body.indexOf("\"serial_debug\":true") >= 0) gState.serialDebug = true;
    else if (body.indexOf("\"serial_debug\":false") >= 0) gState.serialDebug = false;

    if (body.indexOf("\"use_fahrenheit\":true") >= 0) gState.useFahrenheit = true;
    else if (body.indexOf("\"use_fahrenheit\":false") >= 0) gState.useFahrenheit = false;

    if (body.indexOf("\"sensor_enabled\":true") >= 0) {
        setAht20Enabled(true);
    } else if (body.indexOf("\"sensor_enabled\":false") >= 0) {
        setAht20Enabled(false);
    }

    String reqTz = extractJsonStringField(body, "timezone");
    if (reqTz.length() > 0) {
        applyTimeZone(reqTz);
    }

    String reqHost = extractJsonStringField(body, "hostname");
    if (reqHost.length() > 0) {
        applyHostname(reqHost);
    }

    String reqSsid = extractJsonStringField(body, "wifi_ssid");
    String reqPass = extractJsonStringField(body, "wifi_password");
    if (reqSsid.length() > 0) {
        server.send(200, "application/json", "{\"ok\":true}");
        server.handleClient();
        feedWatchdog();
        applyWifiCredentials(reqSsid, reqPass);
        return;
    }

    savePreferences();
    DBG_PRINTF("[WEB] Settings -> host=%s, tz=%s, theme=%s, debug=%s, sensor=%s, unit=%s\n",
               gState.hostname.c_str(),
               gState.timeZone.c_str(),
               gState.theme.c_str(),
               gState.serialDebug ? "ON" : "OFF",
               gState.sensorEnabled ? "ON" : "OFF",
               gState.useFahrenheit ? "F" : "C");

    server.send(200, "application/json", "{\"ok\":true}");
}

static void handleUpdateUpload() {
    HTTPUpload &upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        setWatchdogEnabled(false);
        DBG_PRINTF("[OTA] Start upload: %s (WDT paused)\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
        if (upload.totalSize > 0) {
            int pct = (int)((upload.currentSize * 100ULL) / upload.totalSize);
            DBG_PRINTF("[OTA] Chunk written (%u bytes), total %u bytes (%d%%)\n",
                       upload.currentSize, upload.totalSize, pct);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            DBG_PRINTF("[OTA] Success: %u bytes written. Rebooting...\n", upload.totalSize);
        } else {
            Update.printError(Serial);
            setWatchdogEnabled(true);
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.end();
        setWatchdogEnabled(true);
        DBG_PRINTLN("[OTA] Upload aborted (WDT resumed).");
    }
}

static void handleUpdateComplete() {
    bool ok = !Update.hasError();
    if (ok) {
        server.send(200, "text/plain", "OK");
        delay(250);
        ESP.restart();
    } else {
        setWatchdogEnabled(true);
        server.send(500, "text/plain", "Update failed");
    }
}

void initWebServer() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/status", HTTP_GET, handleStatus);
    server.on("/api/logs", HTTP_GET, handleLogs);
    server.on("/api/logs/clear", HTTP_POST, handleClearLogs);
    server.on("/api/wdt/test", HTTP_POST, handleWdtTest);
    server.on("/api/settings", HTTP_POST, handleSettings);
    server.on("/update", HTTP_POST, handleUpdateComplete, handleUpdateUpload);
    server.begin();
    DBG_PRINTLN("[WEB] HTTP server started on port 80");
}

void handleWebClient() {
    server.handleClient();
}