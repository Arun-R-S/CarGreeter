# 09-storage.md

## 💾 Storage System Overview

The Storage System is responsible for:

* Managing configuration data
* Persisting settings across reboots (optional)

Audio storage is handled by the **JQ6500 module internal flash** in this project.

---

# 🧠 Design Goals

* Reliable file storage
* Low memory usage
* Simple file structure
* Minimal wear on flash memory

---

# 🏗️ Storage Architecture

```text id="f0g3u2"
ESP32-C3 Flash (SPIFFS/LittleFS, optional) → Config Manager (delay, future settings)

JQ6500 Internal Flash → Welcome Audio Track (played by JQ6500)
```

---

# 📦 Storage Options (ESP32-C3)

## 1. Internal Flash (Recommended)

### Options:

* SPIFFS
* LittleFS (Preferred)

---

### Why LittleFS?

* Better reliability
* Wear leveling
* Faster performance

---

## 2. External Flash (Optional)

External flash is not required for welcome audio (stored on JQ6500).

---

# 📁 File Structure

## Basic Layout

```text id="3a0v19"
/config.txt        ← configuration file (delay, future settings)
```

---

## Rules

* Keep config small
* Avoid frequent writes

---

# 📤 File Upload Flow

```text id="d3p8mp"
PC / Tooling
        ↓
Program JQ6500 internal flash
        ↓
Welcome track available for playback
```

---

# ⚙️ Local Storage Module (Optional)

## Responsibility

* Read/write small configuration values
* Keep storage access lightweight and infrequent

---

## Key Functions

* Load config on boot
* Save config only when changed

---

# 🔄 Audio File Handling

## Upload Behavior

* Audio is stored on JQ6500 internal flash
* Update audio via the programming method supported by your JQ6500 module

---

## Playback Behavior

* ESP32-C3 sends a play command
* JQ6500 reads/decodes the track from its internal flash

---

# ⚡ Streaming Support

## Design

* No ESP32-side audio streaming in this design
* ESP32-C3 only controls playback via UART

---

## Benefits

* Low ESP32-C3 RAM usage
* Stable playback handled by JQ6500

---

# 🧾 Configuration Storage

## File

```text id="j1f6pp"
/config.txt
```

---

## Example Content

```text
delay=5
```

---

## Behavior

* Loaded on boot
* Updated when changed
* Saved persistently

---

# ⚠️ Constraints

* Limited flash write cycles
* Avoid frequent writes
* Keep files small

---

# 🧠 Memory Considerations

* Keep buffers small
* Avoid large allocations

---

# 🚫 Not Allowed

* Multiple large files (current version)
* Frequent rewriting of config
* Blocking file operations

---

# 🧪 Testing Guidelines

* Verify config persistence after reboot

---

# 🔄 Error Handling

* Config read failure → use defaults and log warning
* Config write failure → log warning

---

# 🚀 Future Enhancements

* Track selection and volume stored in config
* Optional: switch to I2S-based design if browser audio upload is required

---

# 🏁 Summary

The Storage System provides:

* Persistent configuration storage (optional)
* Clear separation: config on ESP32-C3, audio on JQ6500

---

END OF FILE
