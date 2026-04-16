# 05-web-ui.md

## 🌐 Web UI Overview

The system provides a **browser-based user interface** served directly from the ESP32 firmware.

---

# 🧠 Design Approach

The UI follows a **Tasmota-style embedded approach**:

* HTML, CSS, and JavaScript are stored inside firmware
* Served using `PROGMEM`
* No external files or filesystem dependency required

---

## 🎯 Goals

* Simple and lightweight UI
* Fast loading
* Minimal memory usage
* No external dependencies

---

# 🏗️ UI Architecture

```text id="b3s6o7"
Browser (User)
     ↓
HTTP Request
     ↓
ESP32 Web Server
     ↓
Embedded HTML (web_pages.h)
     ↓
JavaScript → API Calls
```

---

# 📄 UI Pages

## 1. Main Page (`/`)

### Purpose

* Control system
* Configure delay
* Trigger playback
* Manage WiFi and hotspot
* Configure audio settings
* System maintenance (restart, factory reset)
* Backup and restore configuration

---

### Features

* Delay configuration input
* Play button
* WiFi scanning and connection
* Hotspot (AP) configuration
* Audio volume and track selection
* System controls and configuration
* Backup/restore buttons
* Admin credential configuration
* Navigation links to other pages

---

## 2. Log Viewer Page (`/logview`)

### Purpose

* Display system logs in real-time
* Historical log viewing

---

### Features

* Auto-refresh logs every 1.5 seconds
* Manual refresh button
* Scrollable log output
* Copy-to-clipboard functionality
* Circular buffer (25 most recent entries)

---

## 3. System Information Page (`/sysinfo`)

### Purpose

* Display comprehensive system health and status

---

### Features

* **Hardware Section**
  - Chip model (ESP32-C3)
  - CPU core count
  - CPU frequency (MHz)
  - Chip revision number
  - Built-in features (WiFi)

* **Memory Section**
  - Total heap size (327680 bytes)
  - Free heap memory (current)
  - Used heap memory (current)
  - Heap utilization percentage with visual progress bar
  - Minimum free heap recorded

* **Flash Section**
  - Total flash memory size

* **Network Section**
  - WiFi mode (station/AP)
  - Connection status (connected/disconnected)
  - Signal strength (RSSI in dBm with emoji indicators)
  - MAC address
  - IP address
  - Gateway address
  - Subnet mask
  - DNS servers (primary and secondary)

* **System Section**
  - Uptime (human-readable format: "X days, Y hours, Z minutes")
  - Uptime in seconds
  - Active task count

* **Auto-Refresh**: Updates every 3 seconds

---

# 🔗 Navigation Flow

```text id="cztqhl"
/ (Main Page)
   ├→ /logview (Logs Page)
   └→ /sysinfo (System Information Page)
```

---

# 🔧 UI Actions → API Mapping

| UI Section         | UI Action              | API Endpoint          |
| ------------------ | ---------------------- | --------------------- |
| Audio Control      | Play Audio             | GET /play             |
| Delay Settings     | Set Delay              | GET /setDelay         |
| Log Viewer         | View Logs              | GET /logs             |
| System Info        | View System Details    | GET /api/sysinfo      |
| WiFi Config        | Scan Networks          | GET /api/wifi/scan    |
| WiFi Config        | Connect to Network     | POST /api/wifi/connect |
| WiFi Config        | Forget Network         | POST /api/wifi/forget |
| Hotspot Config     | Set Hotspot Settings   | POST /api/hotspot     |
| Admin Credentials  | Update Admin Password  | POST /api/auth        |
| Audio Settings     | Configure Audio        | POST /api/audio       |
| System Control     | Restart Device         | POST /api/restart     |
| System Control     | Factory Reset          | POST /api/factory_reset |
| Backup             | Download Backup        | GET /api/backup       |
| Restore            | Upload Settings        | POST /api/restore     |
| Time Sync          | Sync Time with Browser | POST /api/time/sync   |

---

# 🎨 UI Design Guidelines

## Styling

* Minimal CSS
* Dark theme (optional but recommended)
* Monospace font for logs

---

## Layout

* Simple vertical layout
* Large buttons for usability
* Mobile-friendly

---

# 📦 HTML Storage

All HTML must be stored in:

```text id="fntj0x"
web_pages.h
```

---

## Example Pattern

```cpp
const char MAIN_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
...
</html>
)rawliteral";
```

---

# ⚡ JavaScript Behavior

## Communication Method

* Use `fetch()` API
* Send HTTP requests to ESP32

---

## Example

```javascript
fetch('/play')
  .then(res => res.text())
  .then(data => console.log(data));
```

---

# 🔄 Dynamic Updates

## Logs Page

* Uses periodic polling
* Fetch logs every 1 second

---

## Optional Future Upgrade

* WebSocket-based real-time updates

---

# 🔐 Authentication Behavior

* Browser shows login popup automatically
* No login page required
* Credentials handled by browser

---

# ⚠️ Constraints

* Keep HTML size small
* Avoid heavy JavaScript
* Avoid large inline CSS
* Minimize DOM updates

---

# 🚫 Not Allowed

* External CDN (no internet dependency)
* Large frameworks (React, Angular, etc.)
* Heavy animations

---

# 🧪 Testing Guidelines

* Test on mobile browser
* Test on desktop browser
* Verify all buttons trigger correct API
* Verify logs work correctly

---

# 🚀 Future Enhancements

* Better UI styling
* Playback status indicator
* Playback status indicator
* Config dashboard

---

# 🏁 Summary

The Web UI is:

* Embedded inside firmware
* Lightweight and fast
* Controlled via simple HTTP APIs
* Designed for reliability over complexity

---

END OF FILE
