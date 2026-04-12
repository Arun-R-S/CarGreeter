# 08-logging.md

## 🧾 Logging System Overview

The Logging System provides a **centralized, non-blocking mechanism** to record system activity for debugging, monitoring, and diagnostics.

---

# 🧠 Design Goals

* Non-blocking logging
* Minimal memory usage
* Centralized logging control
* Multi-output support (Serial + Web UI)
* Easy debugging without USB

---

# 🏗️ Logging Architecture

```text id="4r4d6r"
Module → Logger API → Log Queue → Logger Task → Outputs
                                           ├─ Serial
                                           └─ Web Buffer
```

---

# 🔄 Logging Flow

```text id="l7s7xg"
Module generates log
      ↓
Logger API called
      ↓
Log added to queue
      ↓
Logger Task processes log
      ↓
Outputs (Serial + Web UI)
```

---

# 🧩 Logger Module

## Responsibility

* Accept log messages
* Queue logs
* Process logs asynchronously
* Distribute logs to outputs

---

# 📚 Log Levels

| Level | Description           |
| ----- | --------------------- |
| INFO  | General system events |
| WARN  | Potential issues      |
| ERROR | Critical failures     |

---

## Example

```text id="7kqz4y"
[INFO] System started
[WARN] File not found
[ERROR] Playback failed
```

---

# 📦 Log Structure

```text id="q4x0dn"
LogEntry {
    level
    tag
    message
}
```

---

## Example

```text id="o2qg7r"
[Audio] Playback started
[Web] Request received
```

---

# ⚙️ Log Queue

## Implementation

* FreeRTOS Queue

---

## Configuration

```text id="5knx0l"
Queue Size: 20–50 entries
```

---

## Behavior

* Non-blocking write
* If full → log is dropped (acceptable)

---

# 🧵 Logger Task

## Responsibility

* Continuously read from queue
* Format logs
* Send to outputs

---

## Task Flow

```text id="3hjjr5"
Wait for log
     ↓
Format log
     ↓
Send to Serial
     ↓
Store in buffer
```

---

# 💾 Web Log Buffer

## Purpose

* Store recent logs for UI display

---

## Design

* Circular buffer
* Fixed size (e.g., 20–30 logs)

---

## Behavior

* Overwrites oldest logs
* Fast access

---

# 🌐 Log Viewer Integration

## Endpoint

```text id="xkig7p"
/logs
```

---

## Response

* Plain text log list

---

## UI Page

```text id="6kgk12"
/logview
```

---

## Features

* Auto-refresh (polling)
* Manual refresh
* Scrollable logs

---

# ⚡ Non-Blocking Design

## Key Rules

* Logging must never block execution
* Use queue for decoupling
* Avoid heavy string operations

---

# 🧠 Usage Pattern

## Correct

```cpp id="0g7p7b"
logInfo("AUDIO", "Playback started");
```

---

## Incorrect

```cpp id="5nq1ld"
Serial.println("Playback started");
```

---

# ⚠️ Constraints

* Keep messages short
* Avoid dynamic memory where possible
* Limit buffer size to save RAM

---

# 🚫 Not Allowed

* Direct Serial logging in modules
* Blocking logging operations
* Large log payloads

---

# 🧪 Testing Guidelines

* Verify logs appear in Serial
* Verify logs appear in web UI
* Test under heavy logging
* Ensure no system slowdown

---

# 🔄 Error Logging

* Errors must always be logged
* Include meaningful messages
* Avoid silent failures

---

# 🚀 Future Enhancements

* Log filtering (by level/tag)
* Download logs
* Persistent log storage (file)
* Remote logging

---

# 🏁 Summary

The Logging System ensures:

* Visibility into system behavior
* Easy debugging without hardware tools
* Non-blocking and efficient logging
* Integration with web UI for monitoring

---

END OF FILE
