# 01-overview.md

## 🎯 Project Overview

The **ESP32-C3 + JQ6500 Audio Greeting System in Car** is a WiFi-enabled embedded system designed to:

* Play a predefined audio file(Welcome note or Greeting audio file) automatically on power-up
* Provide configuration controls (e.g., playback delay)
* Output audio to external stereo systems via AUX
* Offer real-time logging and monitoring via a web UI
* Secure all operations using authentication

---

## 🧠 Key Objectives

* Provide a **simple and reliable audio playback system**
* Enable **wireless configuration and control**
* Ensure **non-blocking and stable operation**
* Maintain **clean, modular, and scalable architecture**

---

## 🚀 Core Features

### 🔊 Audio Playback

* Plays audio automatically after device power ON
* Playback occurs only once per power cycle
* Audio is streamed from storage (no full RAM load)

---

### 🌐 Web-Based Control

* Embedded web UI (served directly from firmware)
* Accessible via browser over WiFi
* No external app required

---

### 📦 Audio Storage (JQ6500)

* Welcome audio is stored on the JQ6500 module internal flash
* ESP32-C3 controls playback via UART commands (no audio streaming)

---

### ⏱ Configurable Delay

* Set delay before playback
* Stored persistently
* Configurable via UI

---

### 📊 Logging System

* Centralized logging module
* View logs via web interface
* Supports log levels (INFO, WARN, ERROR)

---

### 📺 Log Viewer UI

* Dedicated web page for logs
* Auto-refresh capability
* Debug system without serial monitor

---

### 🔐 Authentication

* Basic Authentication (username/password)
* Protects all critical endpoints
* Prevents unauthorized access

---

## 🏗️ System Behavior

### 🔄 Boot Flow

1. Device powers ON
2. System initializes modules
3. Configuration is loaded
4. Scheduler starts delay timer
5. After delay → playback event triggered
6. Audio plays once
7. System remains idle and responsive

---

### 🌐 User Interaction Flow

1. User connects to device IP via browser
2. Authentication prompt appears
3. User logs in
4. User can:

   * Set delay
   * Trigger playback manually
   * View logs

---

## 🔌 Hardware Overview

### 🧠 Microcontroller

* ESP32-C3 Super Mini (HW-466AB) (with built-in WiFi)

---

### 💾 Storage

* ESP32-C3 flash: configuration storage (delay, future settings)
* JQ6500 internal flash: welcome audio track (16Mbit / 2MB)

---

### 🔊 Audio Output

* UART control from ESP32-C3 → JQ6500
* JQ6500 analog DAC outputs (DACL/DACR) to AUX (line-level)

---

### 🔌 Power

* 5V input with 3.3V regulation
* Stable power required for audio quality

---

## 📡 Connectivity

* WiFi-based access
* Works within local network
* No internet dependency required

---

## ⚙️ Technology Stack

* Arduino Framework (ESP32-C3)
* FreeRTOS (built into ESP32)
* Embedded Web Server
* UART control to JQ6500

---

## 📚 Documentation Map

Complete system documentation is organized in the `/docs` folder:

| Topic                | File                                      | Purpose                                |
| -------------------- | ----------------------------------------- | -------------------------------------- |
| Overview             | [📄 01-overview.md](01-overview.md)      | High-level project description         |
| Architecture         | [📄 02-architecture.md](02-architecture.md) | System design and data flow            |
| Modules              | [📄 03-modules.md](03-modules.md)        | Module responsibilities & interfaces  |
| Event System         | [📄 04-event-system.md](04-event-system.md) | Inter-module communication            |
| Web UI               | [📄 05-web-ui.md](05-web-ui.md)          | User interface pages & actions        |
| Authentication       | [📄 06-authentication.md](06-authentication.md) | Security & access control           |
| Audio System         | [📄 07-audio-system.md](07-audio-system.md) | JQ6500 audio playback                |
| Logging              | [📄 08-logging.md](08-logging.md)        | Log architecture & implementation     |
| Storage              | [📄 09-storage.md](09-storage.md)        | Configuration & backup system         |
| Scheduler            | [📄 10-scheduler.md](10-scheduler.md)    | Boot delay & timing logic             |
| API Specification    | [📄 11-api-spec.md](11-api-spec.md)      | Complete endpoint documentation       |
| Constraints          | [📄 12-non-functional.md](12-non-functional.md) | Performance & design rules          |
| Future Enhancements  | [📄 13-future-enhancements.md](13-future-enhancements.md) | Roadmap & planned features         |
| Build & Deployment   | [📄 14-build-deployment.md](14-build-deployment.md) | Setup & deployment guide           |

---

## 🧩 Design Philosophy

This project is designed with:

* Simplicity in operation
* Reliability in execution
* Flexibility for future expansion

---

## 🚫 Out of Scope (Current Version)

* Bluetooth audio playback
* Multi-user authentication
* Cloud integration
* HTTPS encryption

(These may be added in future versions)

---

## 🏁 Summary

This system provides a **self-contained, configurable, and secure audio playback solution** using ESP32, combining:

* Embedded systems
* Web technologies
* Event-driven architecture

---

END OF FILE
