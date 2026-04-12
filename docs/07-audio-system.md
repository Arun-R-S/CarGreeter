# 07-audio-system.md

## 🔊 Audio System Overview

The Audio System is responsible for **playing audio files stored on the device** and outputting them through an external audio system via **I2S → DAC → AUX**.

---

# 🧠 Design Goals

* Reliable playback on power-up
* Non-blocking audio streaming
* Low memory usage
* Clean integration with event system

---

# 🏗️ Audio Pipeline

```text id="j3p4yk"
Storage → File Manager → Audio Engine → I2S → DAC → AUX Output
```

---

# 🔌 Hardware Components

## 🧠 ESP32

* Provides I2S interface
* Handles audio data streaming

---

## 🎧 DAC (Digital-to-Analog Converter)

Recommended:

* MAX98357A (I2S DAC + amplifier)

---

## 🔊 Output

* Line-level AUX output
* Connect to external stereo system

---

# 📁 Audio File Format

## Recommended Format

* WAV (PCM, 16-bit)
* Sample Rate: 44.1kHz or 22.05kHz

---

## Why WAV?

* No decoding required
* Lightweight processing
* Best suited for ESP32

---

# ⚙️ Audio Engine Module

## Responsibility

* Handle playback logic
* Stream audio data
* Control I2S interface

---

## Input

* EVENT_PLAY

---

## Output

* Audio signal via DAC

---

# 🔄 Playback Flow

```text id="d6ek0s"
EVENT_PLAY received
      ↓
Audio Engine starts playback
      ↓
File Manager opens file
      ↓
Read chunk → send to I2S
      ↓
Repeat until file ends
      ↓
Stop playback
```

---

# 📦 Streaming Design

## Chunk-Based Reading

* Read small chunks (e.g., 512–1024 bytes)
* Send to I2S buffer
* Repeat until EOF

---

## Why Streaming?

* Avoids loading entire file into RAM
* Ensures low memory usage

---

# ⚡ Non-Blocking Strategy

* Playback runs in separate task
* No blocking calls in main loop
* Uses buffers and continuous streaming

---

# 🧵 Audio Task

## Responsibilities

* Wait for EVENT_PLAY
* Execute playback
* Manage I2S data flow

---

## Task Flow

```text id="ik7qpr"
Wait for event
     ↓
Start playback
     ↓
Stream audio
     ↓
End playback
```

---

# 🔊 I2S Configuration

## Key Parameters

* Sample Rate: 44100 Hz (recommended)
* Bits per sample: 16-bit
* Channel: Mono or Stereo

---

## Output Pins (Example)

```text id="m8h7bx"
BCLK → GPIO26
LRCK → GPIO25
DATA → GPIO22
```

(Note: Pins can be configured as needed)

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

* MP3/AAC support
* Volume control
* Pause/Resume
* Playlist support
* Bluetooth audio

---

# 🏁 Summary

The Audio System provides:

* Efficient streaming playback
* Low memory usage
* Clean integration with event-driven architecture
* Reliable output to external audio systems

---

END OF FILE
