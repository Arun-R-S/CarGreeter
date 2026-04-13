# 02-architecture.md

## 🏗️ System Architecture

This project follows a **modular, event-driven, non-blocking architecture** designed for embedded systems like **ESP32-C3**, with audio playback handled by a **JQ6500** module.

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
JQ6500  Config    Config     Logger     Scheduler
Player  Manager   Storage    Task         Task
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
Scheduler → EVENT_PLAY → JQ6500 Player → UART → JQ6500 → DACL/DACR → AUX Output
```

---

## 3. Audio Update Flow

Audio is stored on the JQ6500 module internal flash.

```text
PC / Tooling → Program JQ6500 flash → Welcome track available for playback
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

* JQ6500 Player (audio control)
* Config Manager (configuration)
* Scheduler (boot delay & timing)
* System Manager (metrics collection)
* Network Manager (connectivity)
* Time Manager (real-time clock)

### Responsibility:

* Execute system logic
* Respond to events
* Provide data to Web Server

---

## 5. Infrastructure Layer

### Components:

* Logger
* Storage system
* Hardware drivers (UART)

### Responsibility:

* Handle low-level operations

---

# ⚙️ Task-Based Architecture (FreeRTOS)

The system uses multiple tasks to ensure **non-blocking execution**.

---

## 🧵 Tasks Overview

| Task              | Responsibility                   |
| ----------------- | -------------------------------- |
| Web Server Task   | Handle HTTP requests & responses |
| JQ6500 Task       | Audio control (UART)             |
| Logger Task       | Process logs to outputs          |
| Scheduler Task    | Handle boot delay & timing logic |
| Config Task       | Manage persistent settings       |
| Network Task      | Monitor WiFi connectivity        |
| System Task       | Gather system metrics            |
| Time Task         | Maintain real-time clock         |
| Event Bus Task    | Distribute events to consumers   |

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
| JQ6500 Player  | Event Bus          |
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
