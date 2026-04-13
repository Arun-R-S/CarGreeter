# 10-scheduler.md

## ⏱ Scheduler Overview

The Scheduler Module is responsible for:

* Handling **boot-time delay**
* Triggering **automatic audio playback**
* Ensuring playback happens **only once per power cycle**

---

# 🧠 Design Goals

* Non-blocking execution
* Reliable one-time execution
* Configurable delay
* Clean integration with Event System

---

# 🏗️ Scheduler Architecture

```text id="lqgqdf"
System Boot
     ↓
Load Config (delay)
     ↓
Start Timer
     ↓
Wait (non-blocking)
     ↓
Trigger EVENT_PLAY
     ↓
Stop Scheduler
```

---

# 🔄 Boot Flow Integration

```text id="6xjmt3"
Power ON
   ↓
System Init
   ↓
Config Manager loads delay
   ↓
Scheduler starts
   ↓
Delay countdown begins
```

---

# ⚙️ Scheduler Module

## Responsibility

* Track elapsed time
* Trigger playback event
* Ensure single execution

---

## Input

* Delay value from Config Manager

---

## Output

* EVENT_PLAY

---

# 🧵 Scheduler Task

## Task Behavior

```text id="hmpq7a"
Start task
   ↓
Record start time
   ↓
Loop:
    Check elapsed time
    If delay reached:
        Send EVENT_PLAY
        Exit task
```

---

# ⚡ Non-Blocking Design

## Implementation Options

### Option 1: `millis()` (Recommended)

```cpp id="r0n4pe"
if (millis() - startTime >= delayMs)
```

---

### Option 2: FreeRTOS Delay

```cpp id="4cik27"
vTaskDelay(delayMs / portTICK_PERIOD_MS);
```

---

## Preferred Approach

* Use `millis()` for flexibility
* Avoid long blocking delays

---

# 🧠 One-Time Execution Logic

## Requirement

* Playback must happen **only once per power cycle**

---

## Strategy

```text id="k6j1s9"
Boolean flag → executed = false

If delay reached AND executed == false:
    Trigger EVENT_PLAY
    executed = true
```

---

## After Execution

* Scheduler stops or becomes idle

---

# 🔄 Interaction with Other Modules

## Dependencies

* Config Manager (for delay value)
* Event Bus (to trigger playback)
* Logger (for debug logs)

---

## Example Flow

```text id="6ql8jt"
Scheduler → EVENT_PLAY → JQ6500 Player
```

---

# ⚠️ Constraints

* Must not block system
* Must not run repeatedly
* Must handle delay correctly even if system is busy

---

# 🧪 Testing Guidelines

* Set delay (e.g., 5 seconds)
* Power cycle device
* Verify playback occurs after delay
* Ensure playback happens only once
* Test with different delay values

---

# 🔄 Edge Cases

## 1. Delay = 0

* Playback should happen immediately

---

## 2. Invalid Delay

* Use default value
* Log warning

---

## 3. Restart During Delay

* Restart should reset scheduler
* Delay starts again

---

# 🧾 Logging

## Example Logs

```text id="y1akqg"
[Scheduler] Started
[Scheduler] Waiting for 5 seconds
[Scheduler] Triggering playback
```

---

# 🚀 Future Enhancements

* Multiple scheduled events
* Daily/weekly scheduling
* Time-based triggers (RTC)

---

# 🏁 Summary

The Scheduler ensures:

* Automatic playback after boot
* Configurable delay
* One-time execution
* Non-blocking behavior

---

END OF FILE
