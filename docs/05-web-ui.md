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

---

### Features

* Delay configuration input
* Play button
* Navigation to logs page

---

---

## 2. Log Viewer Page (`/logview`)

### Purpose

* Display system logs

---

### Features

* Auto-refresh logs
* Manual refresh button
* Scrollable log output

---

# 🔗 Navigation Flow

```text id="cztqhl"
/ (Main Page)
   ↓
/logview (Logs Page)
```

---

# 🔧 UI Actions → API Mapping

| UI Action    | API Endpoint  |
| ------------ | ------------- |
| Play Audio   | GET /play     |
| Set Delay    | GET /setDelay |
| View Logs    | GET /logs     |

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
