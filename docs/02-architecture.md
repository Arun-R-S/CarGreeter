# 02-architecture.md

## 🏗️ System Architecture

This project follows a **modular, event-driven, non-blocking architecture** designed specifically for embedded systems like ESP32.

---

# 🧠 High-Level Architecture

```text
[ Web UI (Browser) ]
          ↓
     HTTP Requests
          ↓
   Web Server Module
          ↓
   Authentication Layer
          ↓
       Event Bus
          ↓
 ┌─────────┼──────────┬──────────┬──────────┐
 ▼         ▼          ▼          ▼          ▼
Audio   Config    File       Logger     Scheduler
Engine  Manager   Manager     Task         Task
```

---

# 🔄 Core Data Flow

## 1. User Interaction Flow

```text
Browser → HTTP Request → Web Server → Auth Check → Event Bus → Module Action
```

---

## 2. Playback Flow

```text
Scheduler → EVENT_PLAY → Audio Engine → I2S → DAC → AUX Output
```

---

## 3. Upload Flow

```text
Browser → /upload → Web Server → File Manager → Storage
```

---

## 4. Logging Flow

```text
Module → Logger API → Log Queue → Logger Task → Output (Serial + Web)
```

---

# 🧩 Architectural Layers

## 1. Presentation Layer

### Components:

* Web UI (HTML/CSS/JS embedded in firmware)
* Browser interface

### Responsibility:

* User interaction
* Sending HTTP requests

---

## 2. Interface Layer

### Component:

* Web Server Module

### Responsibility:

* Handle HTTP routes
* Validate authentication
* Convert requests into events

---

## 3. Communication Layer

### Component:

* Event Bus

### Responsibility:

* Central communication system
* Decouple modules
* Ensure event-driven flow

---

## 4. Business Logic Layer

### Components:

* Audio Engine
* Config Manager
* File Manager
* Scheduler

### Responsibility:

* Execute system logic
* Respond to events

---

## 5. Infrastructure Layer

### Components:

* Logger
* Storage system
* Hardware drivers (I2S)

### Responsibility:

* Handle low-level operations

---

# ⚙️ Task-Based Architecture (FreeRTOS)

The system uses multiple tasks to ensure **non-blocking execution**.

---

## 🧵 Tasks Overview

| Task            | Responsibility       |
| --------------- | -------------------- |
| Web Server Task | Handle HTTP requests |
| Audio Task      | Audio playback       |
| Logger Task     | Process logs         |
| Scheduler Task  | Handle delay logic   |

---

## 🧠 Task Design Rules

* Each task must have a **single responsibility**
* Tasks must not block each other
* Communication must happen via queues/events

---

# 🔄 Event-Driven Architecture

## Principle:

> Modules do not call each other directly.

Instead:

```text
Module → Event Bus → Target Module
```

---

## Example:

### ❌ Incorrect:

```cpp
audio.play();
```

### ✅ Correct:

```cpp
sendEvent(EVENT_PLAY);
```

---

# 📡 Event Bus Design

* Implemented using FreeRTOS Queue
* Supports asynchronous communication
* Ensures loose coupling

---

## Event Flow:

```text
Producer → Event Queue → Consumer Task
```

---

# ⚡ Non-Blocking Design

## Rules:

* No `delay()` allowed
* Use:

  * `millis()`
  * FreeRTOS `vTaskDelay`
  * Event queues

---

## Example:

### ❌ Blocking:

```cpp
delay(5000);
```

### ✅ Non-blocking:

```cpp
if (millis() - startTime >= 5000)
```

---

# 🔐 Authentication Flow

```text
Request → Auth Check → Allow / Reject
```

* Applied at Web Server level
* Protects all endpoints

---

# 📦 Module Interaction Summary

| Module         | Communicates via   |
| -------------- | ------------------ |
| Web Server     | Event Bus          |
| Audio Engine   | Event Bus          |
| Config Manager | Event Bus          |
| Scheduler      | Event Bus          |
| Logger         | Direct API + Queue |

---

# 🧠 Design Benefits

## ✅ Loose Coupling

* Modules independent
* Easy to modify or replace

---

## ✅ Scalability

* Add new features via events
* No major refactoring needed

---

## ✅ Stability

* Non-blocking ensures smooth operation
* Tasks isolate failures

---

## ✅ Maintainability

* Clear separation of concerns
* Easy debugging

---

# 🚀 Future Extension Support

Architecture supports:

* Bluetooth audio module
* OTA updates
* Multi-file playback
* Advanced scheduling

---

# 🏁 Summary

This architecture ensures:

* Clean separation of concerns
* Event-driven communication
* Non-blocking execution
* Scalability for future enhancements

---

END OF FILE
