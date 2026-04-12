# 03-modules.md

## 🧩 Module Design Overview

The system is divided into **independent modules**, each following the **Single Responsibility Principle (SRP)**.

Each module:

* Has one clear responsibility
* Communicates only via the Event Bus (except Logger)
* Can be modified or replaced independently

---

# 🧠 Module List

| Module         | Responsibility             |
| -------------- | -------------------------- |
| Web Server     | Handle HTTP requests       |
| Auth Manager   | Handle authentication      |
| Event Bus      | Inter-module communication |
| Audio Engine   | Audio playback             |
| File Manager   | File storage & access      |
| Config Manager | Configuration management   |
| Scheduler      | Boot delay & timed actions |
| Logger         | Logging system             |

---

# 🌐 1. Web Server Module

## Responsibility

* Handle all HTTP routes
* Parse incoming requests
* Send responses
* Trigger events (no business logic)

---

## Key Rules

* Must NOT contain business logic
* Must NOT directly call other modules
* Must only trigger events

---

## Example Actions

* `/play` → EVENT_PLAY
* `/setDelay` → EVENT_SET_DELAY
* `/upload` → handled with File Manager

---

## Dependencies

* Auth Manager
* Event Bus

---

# 🔐 2. Auth Manager Module

## Responsibility

* Validate HTTP authentication
* Protect routes

---

## Key Rules

* Must be lightweight
* Used by Web Server only

---

## Implementation

* Uses Basic Authentication

---

## Dependencies

* WebServer library

---

# 🔄 3. Event Bus Module

## Responsibility

* Central communication system
* Pass events between modules

---

## Key Rules

* All modules must use Event Bus
* No direct module-to-module calls allowed

---

## Implementation

* FreeRTOS Queue

---

## Event Structure

```text id="u0q2gk"
Event {
  type
  value (optional)
}
```

---

# 🔊 4. Audio Engine Module

## Responsibility

* Play audio via I2S
* Handle audio streaming

---

## Key Rules

* Must not block execution
* Must stream audio (no full file load)

---

## Input

* EVENT_PLAY

---

## Output

* Audio signal via DAC

---

## Dependencies

* File Manager
* I2S driver
* Logger

---

# 📁 5. File Manager Module

## Responsibility

* Store and retrieve files
* Handle upload operations

---

## Key Rules

* Must not handle UI logic
* Must not block system

---

## Functions

* Save file
* Read file (stream)
* Delete/replace file

---

## Dependencies

* SPIFFS / LittleFS / External storage
* Logger

---

# ⚙️ 6. Config Manager Module

## Responsibility

* Manage system configuration
* Store persistent settings

---

## Example Config

```text id="mlw04a"
delay = 5 seconds
```

---

## Key Rules

* Must persist data
* Must be lightweight

---

## Input

* EVENT_SET_DELAY

---

## Dependencies

* File Manager
* Logger

---

# ⏱ 7. Scheduler Module

## Responsibility

* Handle boot-time delay
* Trigger playback once

---

## Key Rules

* Must be non-blocking
* Must run only once per power cycle

---

## Flow

```text id="trh2o2"
Boot → Wait → EVENT_PLAY → Exit
```

---

## Dependencies

* Config Manager
* Event Bus
* Logger

---

# 🧾 8. Logger Module

## Responsibility

* Central logging system

---

## Key Rules

* All logs must go through Logger
* Must be non-blocking

---

## Features

* Log levels (INFO, WARN, ERROR)
* Queue-based processing
* Serial output
* Web UI buffer

---

## Dependencies

* FreeRTOS Queue

---

# 🔗 Module Interaction Rules

## Allowed

```text id="6r6x1g"
Module → Event Bus → Module
Module → Logger
```

---

## Not Allowed

```text id="3k7h3q"
Module → Module (direct call)
Web Server → Audio Engine
```

---

# 🧠 Dependency Overview

```text id="7if82z"
Web Server → Auth Manager → Event Bus
Event Bus → All Modules
Audio Engine → File Manager
Config Manager → File Manager
All Modules → Logger
```

---

# ⚠️ Important Constraints

* Modules must be independent
* No circular dependencies
* No shared global state (unless necessary)
* Keep memory usage minimal

---

# 🚀 Extension Strategy

To add a new feature:

1. Create new module (if needed)
2. Define new events
3. Update Event Bus usage
4. Update documentation

---

# 🏁 Summary

This modular design ensures:

* Clean separation of responsibilities
* Easy debugging
* Scalability
* Maintainability

---

END OF FILE
