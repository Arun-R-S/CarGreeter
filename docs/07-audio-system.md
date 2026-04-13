# 07-audio-system.md

## 🔊 Audio System Overview

The Audio System is responsible for **playing a welcome audio track using a JQ6500 voice/MP3 module**.

* ESP32-C3 triggers playback via the Event Bus
* JQ6500 plays audio from its **internal flash storage**
* Audio output is analog via the JQ6500 DAC pins (DACL/DACR) to AUX

---

# 🧠 Design Goals

* Reliable playback on power-up
* Non-blocking playback control
* Low memory usage
* Clean integration with event system

---

# 🏗️ Audio Pipeline

```text id="j3p4yk"
ESP32-C3 → UART → JQ6500 Module → DACL/DACR → AUX Output
                 (internal flash)
```

---

# 🔌 Hardware Components

## 🧠 ESP32-C3

* Provides WiFi + web control + scheduling
* Sends UART control commands to JQ6500 (no audio sample streaming)

---

## 🎧 JQ6500 Module (Decoder + DAC)

* Decodes audio internally
* Produces analog audio outputs (variant-dependent)

---

## 🔊 Output

* AUX/line-level from JQ6500 DAC pins (DACL/DACR + GND)
* Some boards also expose SPK+/SPK- for direct speaker drive (board-dependent)

---

# 📁 Audio File Format

## Recommended Format

Depends on your JQ6500 module/firmware, but common choices are:

* MP3 (typical)
* WAV (on supported variants)

---

## Notes

* ESP32-C3 does not decode audio in this design.
* JQ6500 handles decoding and analog output.

---

# ⚙️ JQ6500 Player Module

## Responsibility

* Handle playback control logic
* Send UART commands to the JQ6500 module

---

## Input

* EVENT_PLAY

---

## Output

* UART command(s) to start playback on JQ6500

---

# 🔄 Playback Flow

```text id="d6ek0s"
EVENT_PLAY received
      ↓
JQ6500 Player sends UART play command
      ↓
JQ6500 plays audio from internal flash
      ↓
Analog output (DACL/DACR) to AUX
```

---

# 📦 Streaming Design

There is no ESP32-side audio streaming in this design.

* ESP32-C3 only sends control commands to JQ6500
* JQ6500 reads and decodes audio from its internal storage

---

# ⚡ Non-Blocking Strategy

* Playback control runs in a separate task/module
* No blocking calls in web handlers
* UART commands are short and event-driven

---

# 🧵 Audio Task

## Responsibilities

* Wait for EVENT_PLAY
* Send UART command(s) to JQ6500 to start playback
* Optionally handle stop/volume/track selection events

---

## Task Flow

```text id="ik7qpr"
Wait for event
     ↓
Start playback
      ↓
JQ6500 plays audio
      ↓
End playback
```

---

# 🔌 UART Configuration (ESP32-C3 → JQ6500)

* TX (ESP32-C3) → RX (JQ6500)
* RX (ESP32-C3) ← TX (JQ6500) (optional)
* GND must be common

---

# 🧠 Playback Control

## Behavior Rules

* Playback occurs only once per power cycle (automatic)
* Manual playback allowed via UI
* Ignore repeated play events during active playback (optional)

---

# ⚠️ Constraints

* Audio file size should be reasonable (based on storage)
* Avoid high bitrate formats
* Ensure stable power supply

---

# 🚫 Not Supported (Current Version)

* MP3 decoding (requires additional libraries)
* Streaming from internet
* Multiple simultaneous audio files

---

# 🧪 Testing Guidelines

* Verify audio plays on boot
* Verify manual playback works
* Test with different file sizes
* Ensure no distortion or glitches

---

# 🚀 Future Enhancements

* Track selection via UI (EVENT_PLAY_TRACK)
* Volume control via UI (EVENT_VOLUME_CHANGE)
* Stop/Pause/Resume (if supported by the JQ6500 command set)

---

# 🏁 Summary

The Audio System provides:

* Simple and reliable welcome playback using JQ6500 internal storage
* Low ESP32-C3 CPU/RAM usage (no decoding/streaming)
* Clean integration with event-driven architecture

---

END OF FILE
