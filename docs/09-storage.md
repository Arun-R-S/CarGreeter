# 09-storage.md

## 💾 Storage System Overview

The Storage System is responsible for:

* Managing configuration data
* Persisting settings across reboots

Audio storage is handled by the **JQ6500 module internal flash** in this project.

---

# 🧠 Design Goals

* Reliable file storage
* Low memory usage
* Simple file structure
* Minimal wear on flash memory
* Safe recovery after power loss (settings integrity)

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
NVS (Preferences)
  namespace: car_greeter
  keys: cfgA / cfgB  ← settings slots (A/B)
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
NVS namespace: car_greeter
```

---

## Example Content

```text
settings record (binary, CRC-validated)
```

---

## Behavior

* Loaded on boot
* Updated when changed
* Saved persistently

---

# ðŸ§  Settings System (Tasmota-Style, High Level)

This section defines the target design for “settings”, inspired by Tasmota’s high-level behavior.

## Goals

* Settings are always available in RAM (fast reads).
* Writes are deferred/debounced to reduce flash wear.
* Stored data is integrity-checked (power-loss safe).
* System has clear reset + backup/restore flows.

---

## 1) Load on Boot

On boot, the Settings subsystem must:

* Load persisted settings from flash (preferred: NVS/Preferences on ESP32; filesystem is secondary).
* Validate integrity:
  * magic + version
  * CRC/checksum
* If invalid / missing:
  * Use defaults
  * Log a warning
  * Continue boot without blocking
* Publish “effective settings” to the system using events (example: emit `EVENT_SET_DELAY` with the loaded/default delay).

---

## 2) Save on Change (Deferred)

Settings must be saved only when needed:

* Web/API handlers MUST NOT do direct persistence writes.
* A Settings task/module receives events (e.g. `EVENT_SET_DELAY`) and updates the in-RAM snapshot.
* It marks settings “dirty” and schedules a write:
  * debounce (example: write after 2–5 seconds of no further changes)
  * minimum write interval (example: do not write more than once every 30 seconds)
* Only write if the persisted value would actually change.

Power-loss safety (recommended approach):

* Use a two-slot (A/B) commit or monotonic “sequence id” record, so an incomplete write can be detected and the last-good copy used.

---

## 3) Reset Behavior

The system must support a “factory reset” that:

* Clears persisted settings (NVS namespace or settings file(s)).
* Restores defaults in RAM.
* Forces a predictable recovery network mode on next boot (example: start hotspot/AP).
* Logs the action.

Reset triggers (choose one or both):

* Hardware: long-press reset button / GPIO.
* Software: authenticated endpoint that emits a reset event (no business logic in the handler).

Safety rules:

* Reset endpoints MUST require Basic Auth.
* Require explicit confirmation (e.g., `confirm=YES`) to prevent accidental resets.

---

## 4) Backup / Restore

Backup/restore is for moving settings between devices or recovery:

### Backup Format (Plain Text)

The backup is a plain-text file with key=value pairs (one per line):

```
delaySeconds=5
volume=30
preloadedIndex=0
customIndex=1
wifiSsid=MyWiFi
wifiPassword=MyPassword123
apSsid=CarGreeter-AP
apPassword=APPassword456
adminUsername=admin
adminPassword=1234
```

### Backup Download

* Authenticated endpoint returns settings as plain text
* File is downloaded with name: `car_greeter_backup_YYYY-MM-DD.txt`
* Includes all configuration including WiFi and hotspot credentials

### Restore Upload

* Authenticated endpoint accepts a plain-text file
* Parses key=value pairs
* Validates schema and bounds (delay range, volume range, password requirements)
* Applies settings to in-RAM configuration
* Persists to NVS
* May trigger device restart to apply changes

### Security Considerations

* Backup file includes credentials (WiFi passwords, hotspot password, admin credentials)
* Store backup files securely
* Only restore backups from trusted sources
* Plain-text format allows manual editing before restore

---

## Recommended Backend (ESP32)

For small settings:

* Prefer **NVS (Preferences)** for key/value settings (wear-leveled, robust, no filesystem mount).
* Use filesystem (LittleFS/SPIFFS) only for larger files or human-editable assets.

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

* Persistent configuration storage
* Clear separation: config on ESP32-C3, audio on JQ6500

---

END OF FILE
