# 🧠 Reusable IoT Framework Core Guide

This folder contains the **Core Concepts** of the CarGreeter platform. These modules are specifically designed to be **loosely coupled** and **hardware-agnostic**, meaning you can use them in any other ESP32 project without modification.

---

## 🏗️ Architecture Philosophy

The core follows three primary rules:
1.  **Event Driven**: Modules communicate ONLY via the Event Bus. No module calls another module's functions.
2.  **Non-Blocking**: No `delay()` is used. Everything runs in FreeRTOS tasks or using `millis()`.
3.  **Independence**: Core modules do NOT depend on "App" logic.

---

## 📡 1. Event Bus (`event_bus.cpp/h`)

The heart of the system. It uses a FreeRTOS queue to dispatch events to subscribers.

### How to Implement:
```cpp
// 1. Initialize and start the task
eventBusInit();
eventBusStartTask();

// 2. Register a handler
eventBusRegisterHandler(EVENT_WIFI_SET, myHandlerFunc, nullptr);

// 3. Send an event
eventBusSend(EVENT_PLAY, 0);
eventBusSendText(EVENT_WIFI_SET, 0, "SSID", "PASS");
```

### How to Extend:
*   Add new event types to the `EventType` enum in `event_bus.h`.
*   Increase `kMaxSubscribersPerEvent` in `event_bus.cpp` if more than 4 modules need to listen to the same event.

---

## 📝 2. Logger (`logger.cpp/h`)

A non-blocking logging engine that outputs to Serial and a web-accessible circular buffer.

### How to Implement:
```cpp
loggerInit();
loggerStartTask();

logInfo("TAG", "Message");
logWarn("TAG", "Warning!");
logError("TAG", "Failure!");
```

### How to Extend:
*   Modify `kMaxLogLines` or `kLineMaxLen` in `logger.cpp` to change how much history is stored for the Web UI.

---

## 🔐 3. Auth Manager (`auth_manager.cpp/h`)

A simple, robust Basic Authentication layer for `WebServer.h`.

### How to Implement:
```cpp
authManagerInit("admin", "password");

// In your web route:
void handleRoute() {
  if (!authManagerEnsure(g_server)) return; // Automatically sends 401 if unauth
  g_server.send(200, "text/plain", "Secret Data");
}
```

---

## 📡 4. Network Manager (`network_manager.cpp/h`)

Handles background WiFi scanning and state management.

### How to Implement:
```cpp
networkManagerInit();
networkManagerStartTask();

// Request a scan via Event Bus
eventBusSend(EVENT_WIFI_SCAN, 0);

// Get results as JSON for your Web UI
char buf[1024];
networkManagerCopyScanJson(buf, sizeof(buf));
```

---

## ⚙️ 5. System Manager (`system_manager.cpp/h`)

Manages low-level system actions like delayed reboots and metrics.

### How to Implement:
```cpp
systemManagerInit();
systemManagerStartTask();

// Trigger a reboot in 2 seconds
eventBusSend(EVENT_SYSTEM_RESTART, 2000);
```

### How to Extend:
*   Add more system-level commands (e.g., Deep Sleep) to the `taskFn` loop.

---

## ⏰ 6. Time Manager (`time_manager.cpp/h`)

A high-resolution wall clock that manages UTC and local (IST) time without an RTC chip.

### How to Implement:
```cpp
timeManagerInit(initialEpochMs);
timeManagerStartTask();

// Sync via browser or NTP
eventBusSendText(EVENT_TIME_SYNC, 0, "1713240000000", nullptr);

// Get current time
uint64_t now = timeManagerNowEpochMsUtc();
```

---

## 🚀 Porting to a New Project

To use this framework in a new project:
1.  Create a `src` folder in your new Arduino sketch.
2.  Copy this `core` folder into it.
3.  In your `.ino` file, `#include "src/core/event_bus.h"` etc.
4.  Initialize all modules in `setup()` as shown in `CarGreeter.ino`.

**Note**: The core is designed for **ESP32-C3** but is compatible with all ESP32 variants using the Arduino framework.
