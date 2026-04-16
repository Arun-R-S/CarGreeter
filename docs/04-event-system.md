# 04-event-system.md

## 🔄 Event System Overview

The system uses an **event-driven architecture** where all module interactions happen through a **central Event Bus**.

---

# 🧠 Core Principle

> ❗ Modules MUST NOT call each other directly
> ✅ All communication MUST go through the Event Bus

---

# 🏗️ Event Flow

```text
Event Producer → Event Queue → Event Consumer
```

---

# 📦 Event Bus Implementation

* Built using **FreeRTOS Queue**
* Supports asynchronous communication
* Ensures non-blocking system behavior

---

# 🧩 Event Structure

Each event follows a simple structure:

```text
Event {
    type        // Event type (enum)
    value       // Optional integer value
}
```

---

## 🧾 Example

```text
EVENT_SET_DELAY → value = 5
EVENT_PLAY → value = 0
```

---

# 📚 Event Types

## 🎯 Core Control Events

| Event           | Value | Description                |
| --------------- | ----- | -------------------------- |
| EVENT_PLAY      | 0     | Trigger audio playback     |
| EVENT_STOP      | 2     | Stop audio playback        |

---

## ⏱ Configuration Events

| Event             | Value | Description              |
| ----------------- | ----- | ------------------------ |
| EVENT_SET_DELAY   | 1     | Update playback delay    |
| EVENT_VOLUME_CHANGE | 3   | Change audio volume      |
| EVENT_AUTH_SET     | 15  | Update admin credentials |

---

## 📡 WiFi Events

| Event          | Value | Description              |
| -------------- | ----- | ------------------------ |
| EVENT_WIFI_SET | 5     | Configure WiFi (STA)     |
| EVENT_WIFI_FORGET | 6  | Forget WiFi credentials  |
| EVENT_WIFI_SCAN | 12   | Scan available networks  |
| EVENT_HOTSPOT_SET | 7  | Configure hotspot (AP)   |

---

## 🔊 Audio Configuration Events

| Event                   | Value | Description                  |
| ----------------------- | ----- | ---------------------------- |
| EVENT_AUDIO_SET_PRELOADED | 8   | Set preloaded track index    |
| EVENT_AUDIO_SET_CUSTOM    | 9   | Set custom track index       |

---

## ⚙️ System Events

| Event                    | Value | Description          |
| ------------------------ | ----- | -------------------- |
| EVENT_SYSTEM_RESTART     | 10    | Restart device       |
| EVENT_SYSTEM_FACTORY_RESET | 11  | Reset to defaults    |
| EVENT_ERROR              | 4     | System error occurred |

---

## ⏰ Time Synchronization Events

| Event             | Value | Description                      |
| ----------------- | ----- | -------------------------------- |
| EVENT_TIME_SYNC   | 13    | Sync time from external source   |
| EVENT_TIME_SYNC_NTP | 14  | Sync time via NTP                |

---

# 🧵 Event Producers

Modules that **generate events**:

| Module       | Events Produced                                           |
| ------------ | --------------------------------------------------------- |
| Web Server   | EVENT_PLAY, EVENT_SET_DELAY, EVENT_VOLUME_CHANGE, EVENT_AUTH_SET, EVENT_AUDIO_SET_*, EVENT_WIFI_*, EVENT_HOTSPOT_SET, EVENT_SYSTEM_*, EVENT_TIME_SYNC |
| Scheduler    | EVENT_PLAY                                                |
| Network Manager | EVENT_WIFI_SET (on connection)                         |

---

# 🎯 Event Consumers

Modules that **handle events**:

| Module             | Events Consumed                                  |
| ------------------ | ------------------------------------------------ |
| JQ6500 Player      | EVENT_PLAY, EVENT_STOP, EVENT_VOLUME_CHANGE      |
| Config Manager     | EVENT_SET_DELAY, EVENT_AUTH_SET, EVENT_AUDIO_SET_* |
| WiFi/Network       | EVENT_WIFI_SET, EVENT_WIFI_FORGET, EVENT_WIFI_SCAN, EVENT_HOTSPOT_SET |
| System            | EVENT_SYSTEM_RESTART, EVENT_SYSTEM_FACTORY_RESET |
| Time Manager       | EVENT_TIME_SYNC, EVENT_TIME_SYNC_NTP             |

---

# ⚙️ Event Queue Design

## Queue Configuration

```text
Queue Size: Dynamically sized based on event pressure
Type: FIFO (First In First Out)
```

---

## Design Rules

* Must not block when sending events
* If queue is full → event handling may be prioritized
* Queue must be thread-safe (FreeRTOS handles this)

---

# 🔁 Event Lifecycle

```text
1. Event created by producer
2. Event pushed to queue
3. Consumer task receives event
4. Consumer processes event
```

---

# 🔌 Example Flow

## 🎯 Play Audio from Web

```text
User clicks "Play"
      ↓
Web Server
      ↓
sendEvent(EVENT_PLAY)
      ↓
Event Queue
      ↓
JQ6500 Player
      ↓
send UART play command
```

---

## ⏱ Delay Update Flow

```text
User sets delay
      ↓
Web Server
      ↓
sendEvent(EVENT_SET_DELAY, value)
      ↓
Event Queue
      ↓
Config Manager
      ↓
saveDelay(value)
```

---

# ⚡ Non-Blocking Behavior

## Sending Events

* Must be non-blocking
* Use zero timeout or minimal delay

---

## Receiving Events

* Tasks may block waiting for events
* This is acceptable and efficient

---

# 🧠 Design Patterns Used

* Producer–Consumer Pattern
* Message Queue Pattern
* Loose Coupling Architecture

---

# ⚠️ Important Rules

## ❌ Not Allowed

* Direct function calls between modules
* Blocking while sending events
* Complex event structures (keep it simple)

---

## ✅ Required

* Use event system for ALL actions
* Keep event structure minimal
* Handle events quickly

---

# 🔄 Event Handling Strategy

## Single Queue vs Multiple Queues

### Current Design:

* Single central queue

### Reason:

* Simpler design
* Easier debugging
* Lower memory usage

---

## Future Option:

* Multiple queues per module (if needed)

---

# 🧩 Event Processing Model

Each module task:

```text
Loop:
    Wait for event
    If event matches → process
    Else → ignore
```

---

# 🧪 Error Handling

* Invalid events should be ignored safely
* Log unexpected events using Logger

---

# 📊 Performance Considerations

* Keep event structure small
* Avoid large payloads
* Minimize queue size to save memory

---

# 🚀 Extension Guidelines

When adding new features:

1. Define new event type
2. Update producer module
3. Update consumer module
4. Document event in this file

---

# 🏁 Summary

The Event System ensures:

* Loose coupling between modules
* Clean communication flow
* Non-blocking execution
* Scalable architecture

---

END OF FILE
