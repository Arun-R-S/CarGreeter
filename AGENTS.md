# AGENT.md

## 🎯 Project: ESP32-C3 + JQ6500 Audio Greeting System in Car (Event-Driven IoT System)

This project uses an **ESP32-C3 (WiFi)** for web control and scheduling, and a **JQ6500 voice/MP3 module** for audio playback. The firmware follows a **modular, event-driven, non-blocking architecture**.

---

# 🧠 Purpose of This File

This document is the **primary entry point for any AI agent or developer** working on this project.

It defines:

* System architecture rules
* Development constraints
* Navigation to detailed documentation
* Expected coding patterns

All implementation MUST follow this file.

---

# 🧭 Documentation Map

The complete system is defined in `/docs`.

| Area                | File                           |
| ------------------- | ------------------------------ |
| Overview            | docs/01-overview.md            |
| Architecture        | docs/02-architecture.md        |
| Modules             | docs/03-modules.md             |
| Event System        | docs/04-event-system.md        |
| Web UI              | docs/05-web-ui.md              |
| Authentication      | docs/06-authentication.md      |
| Audio System        | docs/07-audio-system.md        |
| Logging             | docs/08-logging.md             |
| Storage             | docs/09-storage.md             |
| Scheduler           | docs/10-scheduler.md           |
| API Specification   | docs/11-api-spec.md            |
| Constraints         | docs/12-non-functional.md      |
| Future Enhancements | docs/13-future-enhancements.md |
| Build & Deployment  | docs/14-build-deployment.md    |

---

# 🏗️ Core Architecture Principles

All code MUST follow these principles:

## 1. Single Responsibility

* Each module must handle only one responsibility
* No mixed concerns

## 2. Modular Design

* Each feature must be implemented in separate `.cpp/.h` files
* Modules must be replaceable

## 3. Event-Driven Architecture

* No direct module-to-module calls
* Communication ONLY via Event Bus

## 4. Non-Blocking Execution

* `delay()` is strictly forbidden
* Use FreeRTOS tasks, queues, or millis()

---

# 🚫 Strict Rules (Do Not Violate)

* ❌ No `delay()` usage
* ❌ No direct calls between modules (e.g., web → audio)
* ❌ No business logic inside web handlers
* ❌ No direct Serial logging (use logger module)
* ❌ No large dynamic memory allocations

---

# ✅ Required Patterns

## Event-Based Communication

All actions must flow through the event system:

Example:
Web Server → EVENT_PLAY → JQ6500 Player

---

## Logging

All logs must go through logger module:

logInfo("AUDIO", "Playback started");

---

## Authentication

All HTTP routes must be protected using Basic Authentication unless explicitly public.

---

# 🧩 System Overview

Web UI → Web Server → Auth Layer → Event Bus → Modules

Modules:

* JQ6500 Player (UART-controlled audio playback)
* Config Manager
* Scheduler
* Logger

---

# ⚙️ Task Model

System uses FreeRTOS tasks:

* Web Server Task
* Audio Task
* Logger Task
* Scheduler Task

Tasks must not block each other.

---

# 📦 Development Workflow

1. Read AGENT.md
2. Identify required module
3. Refer corresponding `/docs/*.md`
4. Implement in `/src`
5. Ensure rules are followed
6. Test via web UI

---

# 🔐 Security Model

* Basic Authentication required
* Credentials configurable (future)
* No open endpoints for control actions

---

# 🧪 Testing Guidelines

* Verify event flow
* Ensure no blocking calls
* Validate memory usage
* Test via browser UI

---

# 🚀 Extension Rules

When adding new features:

* Add new events (do NOT modify existing flow)
* Extend modules instead of modifying core logic

* Stability
* Scalability
* Maintainability

If unsure:
➡️ Follow simplicity
➡️ Follow modularity
➡️ Follow event-driven design

---

END OF FILE
