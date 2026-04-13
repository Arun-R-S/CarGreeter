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

| Method | Endpoint    | Description            |
| ------ | ----------- | ---------------------- |
| GET    | `/`         | Main UI page           |
| GET    | `/logview`  | Log viewer UI          |
| GET    | `/play`     | Trigger audio playback |
| GET    | `/setDelay` | Set playback delay     |
| GET    | `/logs`     | Fetch logs             |

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

# ⏱ 4. GET `/setDelay`

## Description

Sets the playback delay.

---

## Query Parameters

| Param   | Type    | Description      |
| ------- | ------- | ---------------- |
| `value` | integer | Delay in seconds |

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

# 🚀 Future Enhancements

* JSON-based responses
* Status endpoint (`/status`)
* File management APIs
* Config API

---

# 🏁 Summary

The API layer provides:

* Simple control interface
* Clear contract between UI and backend
* Secure and lightweight communication
* Integration with event-driven architecture

---

END OF FILE
