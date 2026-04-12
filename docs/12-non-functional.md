# 12-non-functional.md

## ⚙️ Non-Functional Requirements

This document defines the **constraints, performance expectations, and system rules** that must be followed to ensure a stable and efficient ESP32-based system.

---

# 🧠 Design Philosophy

The system must prioritize:

* Stability over features
* Predictability over complexity
* Efficiency over abstraction

---

# ⚡ Performance Requirements

## 🕒 Responsiveness

* Web UI must respond within **< 500 ms**
* API endpoints must return quickly (non-blocking)

---

## 🔄 Event Processing

* Events must be processed in near real-time
* No event should block the queue

---

## 🔊 Audio Playback

* Playback must be smooth (no glitches)
* Continuous streaming without interruption

---

# 💾 Memory Constraints

## RAM Usage

* ESP32 RAM is limited (~320KB usable)
* Avoid large allocations

---

## Rules

* Do not load entire audio file into memory
* Use small buffers (512–1024 bytes)
* Limit number of global variables

---

## Flash Usage

* Store only required files
* Avoid unnecessary duplication

---

# ⚡ Non-Blocking Requirement (CRITICAL)

## ❌ Forbidden

```cpp
delay(1000);
```

---

## ✅ Allowed

* `millis()`
* FreeRTOS queues
* `vTaskDelay()` (short durations only)

---

## Rule

> No operation should block the main system execution

---

# 🧵 Concurrency Model

## FreeRTOS Usage

* Use tasks for parallel operations
* Use queues for communication

---

## Rules

* Avoid race conditions
* Use mutex if shared resource exists
* Keep tasks lightweight

---

# 🔄 Reliability Requirements

## Boot Behavior

* System must always boot reliably
* No crash loops

---

## Playback Reliability

* Playback must trigger correctly after delay
* Must not repeat unintentionally

---

## Error Handling

* All errors must be logged
* System must recover gracefully

---

# 🔐 Security Constraints

* All endpoints must require authentication
* No open control APIs
* Credentials must not be exposed in logs

---

# 📦 Storage Constraints

* Flash has limited write cycles
* Avoid frequent writes

---

## Rules

* Config writes only when changed
* Avoid continuous logging to file

---

# 🌐 Network Constraints

* Works on local WiFi only
* No internet dependency

---

## Rules

* Must handle temporary network loss gracefully
* Must not crash if WiFi disconnects

---

# 📊 Logging Constraints

* Logging must be non-blocking
* Log size must be limited

---

## Rules

* Use circular buffer
* Drop logs if queue is full

---

# 🔌 Power Constraints

* Must handle stable 5V input
* Avoid power spikes during playback

---

# 📱 UI Constraints

* Must be lightweight
* Must load quickly
* Must work on mobile browsers

---

# 🧪 Testing Requirements

* Test under continuous operation
* Test repeated power cycles
* Test large file upload
* Test edge cases (invalid input)

---

# ⚠️ Failure Handling

## System Must:

* Not crash on invalid input
* Not hang due to blocking calls
* Recover from minor errors

---

# 🚫 Not Allowed

* Blocking delays
* Heavy frameworks
* Large memory allocations
* Tight coupling between modules

---

# 🚀 Scalability Considerations

System must allow:

* Adding new modules
* Adding new events
* Extending UI

Without major redesign

---

# 🏁 Summary

These constraints ensure:

* Stable operation
* Efficient resource usage
* Reliable performance
* Long device lifespan

---

END OF FILE
