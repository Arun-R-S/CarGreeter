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

## 🎯 Core Events

| Event             | Description            |
| ----------------- | ---------------------- |
| EVENT_PLAY        | Trigger audio playback |
| EVENT_SET_DELAY   | Update playback delay  |
| EVENT_UPLOAD_DONE | File upload completed  |

---

## 🧪 Optional/Future Events

| Event               | Description               |
| ------------------- | ------------------------- |
| EVENT_STOP          | Stop playback             |
| EVENT_VOLUME_CHANGE | Adjust volume             |
| EVENT_ERROR         | System error notification |

---

# 🧵 Event Producers

Modules that **generate events**:

| Module       | Events Produced             |
| ------------ | --------------------------- |
| Web Server   | EVENT_PLAY, EVENT_SET_DELAY |
| Scheduler    | EVENT_PLAY                  |
| File Manager | EVENT_UPLOAD_DONE           |

---

# 🎯 Event Consumers

Modules that **handle events**:

| Module         | Events Consumed       |
| -------------- | --------------------- |
| Audio Engine   | EVENT_PLAY            |
| Config Manager | EVENT_SET_DELAY       |
| Logger         | (optional monitoring) |

---

# ⚙️ Event Queue Design

## Queue Configuration

```text
Queue Size: 10–20 events
Type: FIFO (First In First Out)
```

---

## Design Rules

* Must not block when sending events
* If queue is full → event may be dropped
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
Audio Task
      ↓
playAudio()
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
