# 06-authentication.md

## 🔐 Authentication Overview

The system uses **HTTP Basic Authentication** to secure access to the web interface and API endpoints.

---

# 🧠 Design Choice

Basic Authentication is chosen because it is:

* Simple to implement
* Lightweight for ESP32
* Supported natively by browsers
* Suitable for local network usage

---

# 🔑 Authentication Flow

```text id="qjv5dp"
Client Request
      ↓
Server checks Authorization Header
      ↓
Valid → Allow Request
Invalid → Request Authentication (401)
```

---

# 🌐 Browser Behavior

* Browser automatically shows a login popup
* Credentials are cached by the browser
* Subsequent requests include Authorization header

---

# 🧾 Implementation

## Core Method

```cpp id="s7t4d7"
server.authenticate(username, password)
```

---

## Example

```cpp id="c6xk3c"
if (!server.authenticate("admin", "1234")) {
    return server.requestAuthentication();
}
```

---

# 🔒 Protected Routes

All sensitive endpoints MUST be protected with Basic Authentication.

---

## List of Protected Routes

### UI Pages

| Endpoint    | Description              |
| ----------- | ------------------------ |
| `/`         | Main UI page             |
| `/logview`  | Log viewer UI page       |
| `/sysinfo`  | System information page  |

### Control Endpoints

| Endpoint    | Description              |
| ----------- | ------------------------ |
| `/play`     | Trigger audio playback   |
| `/setDelay` | Configure playback delay |
| `/logs`     | Fetch recent logs        |

### API Configuration

| Endpoint            | Description                    |
| ------------------- | ------------------------------ |
| `/api/settings`     | Get current settings           |
| `/api/sysinfo`      | Get system information         |
| `/api/backup`       | Download settings backup       |
| `/api/restore`      | Restore settings from backup   |
| `/api/wifi/scan`    | Scan available WiFi networks   |
| `/api/wifi/connect` | Connect to WiFi network        |
| `/api/wifi/forget`  | Forget WiFi credentials        |
| `/api/hotspot`      | Configure hotspot (AP)         |
| `/api/audio`        | Configure audio settings       |
| `/api/restart`      | Restart device                 |
| `/api/factory_reset`| Factory reset device           |
| `/api/time/sync`    | Synchronize device time        |

---

## Authentication Failure Behavior

* **Credentials Missing**: Browser shows Basic Auth login popup
* **Invalid Credentials**: HTTP 401 Unauthorized
* **Expired Session**: Credentials are not time-limited; browser caches them

---

# ⚙️ Authentication Module

## Component

* `auth_manager.cpp / .h`

---

## Responsibility

* Validate credentials
* Enforce access control

---

## Usage Pattern

```cpp id="j0c4zk"
if (!checkAuth(server)) return;
```

---

# 🔑 Credential Management

## Current Approach

* Hardcoded username/password

```text id="gq6e3b"
Username: admin
Password: 1234
```

---

## Future Improvement

* Store credentials in config
* Allow change via UI

---

# ⚠️ Security Limitations

## 1. No Encryption

* Credentials are Base64 encoded (not encrypted)
* Can be intercepted on open networks

---

## 2. No Logout Mechanism

* Browser caches credentials
* No explicit logout option

---

## 3. Single User Only

* No multi-user support

---

# 🛡️ Security Best Practices

## Recommended Usage

* Use only on trusted local networks
* Avoid public WiFi usage

---

## Optional Enhancements

* Change default credentials
* Add login attempt limits
* Add session-based auth (future)

---

# 🚫 Not Allowed

* Open access to control endpoints
* Hardcoding multiple credentials across files
* Skipping authentication checks

---

# 🧪 Testing Guidelines

* Verify login prompt appears
* Test invalid credentials
* Test access to all protected routes
* Ensure no endpoint bypasses authentication

---

# 🔄 Authentication Flow Example

```text id="u1e6k1"
User opens http://esp-ip/
        ↓
Browser shows login popup
        ↓
User enters credentials
        ↓
Server validates
        ↓
Access granted → UI loads
```

---

# 🧠 Integration with System

* Authentication is enforced at Web Server layer
* Does not affect Event Bus or internal modules

---

# 🚀 Future Enhancements

* Session-based authentication
* Token-based authentication
* HTTPS support (advanced)

---

# 🏁 Summary

Authentication ensures:

* Controlled access to system
* Protection from unauthorized usage
* Simple and reliable security for embedded environment

---

END OF FILE
