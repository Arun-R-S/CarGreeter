# 11-api-spec.md

## 🌐 API Specification Overview

This document defines all HTTP endpoints exposed by the ESP32-C3 Web Server.

These APIs are used by:

* Embedded Web UI (JavaScript)
* External clients (optional)

---

# 🧠 Design Principles

* Simple REST-like APIs
* Lightweight responses (text/plain or JSON)
* All endpoints protected by Basic Authentication
* Non-blocking request handling

---

# 🔐 Authentication

* All endpoints require Basic Authentication
* Unauthorized requests return:

```text
HTTP 401 Unauthorized
```

---

# 📚 Endpoint Summary

## UI Pages

| Method | Endpoint    | Description                    |
| ------ | ----------- | ------------------------------ |
| GET    | `/`         | Main UI page                   |
| GET    | `/logview`  | Log viewer UI                  |
| GET    | `/sysinfo`  | System information UI          |

## Core Control

| Method | Endpoint      | Description               |
| ------ | ------------- | ------------------------- |
| GET    | `/play`       | Trigger audio playback    |
| GET    | `/setDelay`   | Set playback delay        |
| GET    | `/logs`       | Fetch recent logs (25)    |

## Settings & Configuration

| Method | Endpoint              | Description              |
| ------ | --------------------- | ------------------------ |
| GET    | `/api/settings`       | Get current settings     |
| GET    | `/api/sysinfo`        | Get system information   |
| GET    | `/api/backup`         | Download settings backup |
| POST   | `/api/restore`        | Restore settings         |
| POST   | `/api/auth`           | Set admin credentials    |

## WiFi Management

| Method | Endpoint              | Description           |
| ------ | --------------------- | --------------------- |
| GET    | `/api/wifi/scan`      | Scan WiFi networks    |
| POST   | `/api/wifi/connect`   | Connect to WiFi       |
| POST   | `/api/wifi/forget`    | Forget WiFi config    |

## Hotspot Management

| Method | Endpoint          | Description          |
| ------ | ----------------- | -------------------- |
| POST   | `/api/hotspot`    | Set hotspot (AP) config |

## Audio Settings

| Method | Endpoint       | Description           |
| ------ | -------------- | --------------------- |
| POST   | `/api/audio`   | Set audio preferences |

## System Maintenance

| Method | Endpoint              | Description         |
| ------ | --------------------- | ------------------- |
| POST   | `/api/restart`        | Restart device      |
| POST   | `/api/factory_reset`  | Factory reset       |
| POST   | `/api/time/sync`      | Sync time from browser |

---

# 📄 1. GET `/`

## Description

Returns the main UI page.

---

## Response

* Content-Type: `text/html`
* Body: Embedded HTML

---

# 📄 2. GET `/logview`

## Description

Returns the log viewer UI page.

---

## Response

* Content-Type: `text/html`

---

# 🔊 3. GET `/play`

## Description

Triggers audio playback.

---

## Flow

```text
Request → Event Bus → EVENT_PLAY → JQ6500 Player
```

---

## Response

```text
200 OK
Playback triggered
```

---

## Query Parameters

| Param   | Type    | Description                                      |
| ------- | ------- | ------------------------------------------------ |
| `index` | integer | (Optional) Specific JQ6500 track index to play. |

---

## Example

```text
/play?index=5
```

---

## Flow

```text
Request (index=5) → Event Bus → EVENT_PLAY (val=5) → JQ6500 Player (Direct Play)
```

---

# 🔌 UART Configuration (ESP32-Cam → JQ6500)

* TX (ESP32 GPIO 13) → RX (JQ6500)
* RX (ESP32 GPIO 15) ← TX (JQ6500) (optional for track count detection)
* GND must be common

---

# 🧠 Reliability & Timing

To ensure the JQ6500 module operates reliably, the following logic is implemented:

## 1. Startup Delay
The audio task waits **500ms** after power-up before sending any UART commands. This ensures the JQ6500 internal processor is fully awake.

## 2. Command Gap
A **100ms delay** is enforced between a `setVolume` command and a `playTrack` command. This gives the JQ6500 enough time to process each command.

## 3. Playback Loop Safety
The volume is explicitly re-applied immediately before every playback command to ensure consistency even if the module reset or missed earlier syncs.

---

## Example

```text
/setDelay?value=5
```

---

## Flow

```text
Request → EVENT_SET_DELAY → Config Manager
```

---

## Response

```text
200 OK
Delay updated
```

---

# 📊 5. GET `/logs`

## Description

Returns recent logs.

---

## Response

* Content-Type: `text/plain`

---

## Example

```text
[INFO] System started
[Audio] Playback started
```

---

# ⚙️ Response Format Guidelines

## Success

```text
200 OK
<message>
```

---

## Error

```text
400 Bad Request
<error message>
```

---

## Unauthorized

```text
401 Unauthorized
```

---

# ⚠️ Constraints

* Keep responses small
* Avoid JSON unless needed
* No large payloads

---

# 🚫 Not Allowed

* Blocking operations in handlers
* Direct module calls (must use events)
* Large file responses

---

# 🧪 Testing Guidelines

* Test all endpoints via browser/Postman
* Verify authentication required
* Validate correct responses
* Test invalid inputs

---

# 🔄 Example Request Flow

## Play Audio

```text
GET /play
   ↓
Auth Check
   ↓
sendEvent(EVENT_PLAY)
   ↓
Return 200 OK
```

---

# 📄 6. GET `/sysinfo`

## Description

Returns the system information UI page with comprehensive hardware, memory, network, and runtime details.

---

## Response

* Content-Type: `text/html`
* Auto-refresh every 3 seconds

---

# 📄 7. GET `/api/settings`

## Description

Returns current system settings as JSON.

---

## Response

```json
{
  "delaySeconds": 5,
  "volume": 30,
  "preloadedIndex": 0,
  "customIndex": 1,
  "wifiSsid": "MyWiFi",
  "apSsid": "CarGreeter-AP",
  "adminUsername": "admin"
}
```

---

## Content-Type

`application/json`

---

# 📊 8. GET `/api/sysinfo`

## Description

Returns comprehensive system information as JSON.

---

## Response

```json
{
  "hardware": {
    "chip": "ESP32-C3",
    "cores": 1,
    "cpuFreq": 160,
    "revision": 3,
    "features": ["WiFi"],
    "model": "ESP32-C3 (RISC-V)"
  },
  "memory": {
    "heapTotal": 327680,
    "heapFree": 245812,
    "heapUsed": 81868,
    "heapPercent": 25,
    "minHeapFree": 198144
  },
  "flash": {
    "size": 4194304
  },
  "network": {
    "wifiMode": 1,
    "connected": true,
    "rssi": -55,
    "mac": "84:F7:03:12:34:56",
    "ip": "192.168.1.100",
    "gateway": "192.168.1.1",
    "subnet": "255.255.255.0",
    "dns1": "8.8.8.8",
    "dns2": "8.8.4.4"
  },
  "system": {
    "uptime": "1 day, 5 hours, 23 minutes",
    "uptimeSeconds": 99781,
    "taskCount": 8
  }
}
```

---

## Content-Type

`application/json`

---

# 💾 9. GET `/api/backup`

## Description

Downloads a backup of all system settings and configuration in plain text format.

---

## Query Parameters

None

---

## Response

* Content-Type: `text/plain`
* Filename: `car_greeter_backup_YYYY-MM-DD.txt`

### Backup Format (Plain Text)

```
delaySeconds=5
volume=30
preloadedIndex=0
customIndex=1
wifiSsid=MyWiFi
wifiPassword=MyPassword123
apSsid=CarGreeter-AP
apPassword=APPassword456
adminUsername=admin
adminPassword=1234
```

---

## Notes

* Backup includes WiFi credentials and hotspot password
* Downloaded file can be restored via `/api/restore` endpoint
* All settings are included in plain text for easy manual editing

---

# 🔄 10. POST `/api/restore`

## Description

Restores system settings from a backup file previously created by `/api/backup`.

---

## Request

* Content-Type: `multipart/form-data` or raw text
* Body: Settings in key=value format (one per line)

### Expected Fields

```
delaySeconds=<int>
volume=<int>
preloadedIndex=<int>
customIndex=<int>
wifiSsid=<string>
wifiPassword=<string>
apSsid=<string>
apPassword=<string>
adminUsername=<string>
adminPassword=<string>
```

---

## Response

```
200 OK
Settings restored successfully
```

---

## Error Response

```
400 Bad Request
Invalid settings format
```

---

## Notes

* Parses plaintext key=value format
* Includes WiFi and hotspot password restoration
* Validates settings before applying
* Device may restart after restore

---

# 📡 11. GET `/api/wifi/scan`

## Description

Scans available WiFi networks and returns list of SSIDs.

---

## Response

```json
[
  {"ssid":"Network1","rssi":-45,"encryption":4},
  {"ssid":"Network2","rssi":-65,"encryption":2},
  {"ssid":"Network3","rssi":-78,"encryption":0}
]
```

---

## Notes

* Scans take 1-2 seconds
* RSSI values in dBm (lower = stronger signal, typically -30 to -90)
* Returns up to 16 networks

---

# 🔗 12. POST `/api/wifi/connect`

## Description

Connects to a specified WiFi network.

---

## Request Parameters

| Param      | Type   | Description                |
| ---------- | ------ | -------------------------- |
| `ssid`     | string | WiFi network name          |
| `password` | string | WiFi password              |

---

## Response

```
200 OK
Connected to WiFi
```

---

## Error Response

```
400 Bad Request
Failed to connect to WiFi
```

---

## Notes

* Connection may take 5-10 seconds
* Device will restart WiFi connection if already connected

---

# 🚫 13. POST `/api/wifi/forget`

## Description

Forgets the current WiFi configuration.

---

## Request

No parameters required

---

## Response

```
200 OK
WiFi configuration cleared
```

---

## Notes

* Device will revert to hotspot mode if no WiFi configured

---

# 📡 14. POST `/api/hotspot`

## Description

Configures the WiFi hotspot (Access Point mode).

---

## Request Parameters

| Param      | Type   | Description         |
| ---------- | ------ | ------------------- |
| `ssid`     | string | Hotspot network name |
| `password` | string | Hotspot password    |

---

## Response

```
200 OK
Hotspot updated
```

---

## Notes

* Requires device restart to apply
* Hotspot runs concurrently with WiFi (if configured)
* Default SSID: `CarGreeter-AP`

---

# 🔊 15. POST `/api/audio`

## Description

Configures audio playback settings.

---

## Request Parameters

| Param             | Type    | Description                     |
| ----------------- | ------- | ------------------------------- |
| `volume`          | integer | Volume level (0-100)            |
| `preloadedIndex`  | integer | Preloaded track index           |
| `customIndex`     | integer | Custom (uploaded) track index   |

---

## Response

```
200 OK
Audio settings updated
```

---

## Notes

* Updates Config Manager via EVENT_AUDIO_SET event
* Volume persisted across reboots

---

# 🔄 16. POST `/api/restart`

## Description

Restarts the device.

---

## Request

No parameters required

---

## Response

```
200 OK
Restarting...
```

---

## Notes

* Device will restart immediately
* All connections will be lost
* Settings are preserved (not a factory reset)

---

# 🏭 17. POST `/api/factory_reset`

## Description

Performs a factory reset, clearing all settings to defaults.

---

## Request

No parameters required

---

## Response

```
200 OK
Factory reset initiated
```

---

## Notes

* Use with caution

---

# 🔐 18. POST `/api/auth`

## Description

Sets the administrative credentials for the Web UI.

---

## Request Parameters

| Param      | Type   | Description         |
| ---------- | ------ | ------------------- |
| `username` | string | New admin username  |
| `password` | string | New admin password  |

---

## Response

```
200 OK
Auth saved; restarting...
```

---

## Notes

* Requires device restart to apply
* Replaces previous credentials in NVS
* Changes the Basic Authentication requirements immediately after restart

---

# ⏰ 18. POST `/api/time/sync`

## Description

Synchronizes device time with client (browser) time.

---

## Request Parameters

| Param        | Type    | Description                      |
| ------------ | ------- | -------------------------------- |
| `epochMsUtc` | integer | Current Unix timestamp in milliseconds |

---

## Response

```
200 OK
Time synchronized
```

---

## Notes

* Allows device to maintain accurate time without NTP
* Called automatically by web UI on page load
* Updates internal time reference for logging

---

# ⚙️ Response Format Guidelines

## Success

```text
200 OK
<message>
```

---

## Error

```text
400 Bad Request
<error message>
```

---

## Unauthorized

```text
401 Unauthorized
```

---

# ⚠️ Constraints

* Keep responses small
* Avoid JSON unless needed
* No large payloads
* All endpoints non-blocking

---

# 🚫 Not Allowed

* Blocking operations in handlers
* Direct module calls (must use events)
* Large file responses
* Long-running synchronous operations

---

# 🧪 Testing Guidelines

* Test all endpoints via browser/Postman
* Verify authentication required on all endpoints
* Validate correct responses
* Test invalid inputs
* Test network disconnection scenarios
* Verify backup/restore data integrity

---

# 🔄 Example Request Flow

## Play Audio with Delay Update

```text
1. GET /setDelay?value=5
   ↓
   Auth Check
   ↓
   sendEvent(EVENT_SET_DELAY, {value: 5})
   ↓
   Return: 200 OK

2. GET /play
   ↓
   Auth Check
   ↓
   sendEvent(EVENT_PLAY)
   ↓
   Return: 200 OK
```

---

# 🚀 Real-Time System Monitoring

The `/api/sysinfo` endpoint provides real-time system metrics that can be polled by external monitoring systems:

* CPU frequency and core count
* Memory utilization (free/used/percent)
* Network connectivity status and signal strength (RSSI)
* System uptime and active task count

This enables building custom dashboards and alerting on device health.

---

# 🏁 Summary

The API layer provides:

* **18 endpoints** covering control, configuration, WiFi, audio, and maintenance
* Simple REST-like interfaces with clear request/response contracts
* Comprehensive system information via JSON API
* Backup/restore for full settings persistence
* Battery of time synchronization for accurate logging
* Real-time network and memory metrics
* Secure authentication on all endpoints
* Non-blocking request handling for stability

---

END OF FILE
