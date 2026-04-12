# 09-storage.md

## 💾 Storage System Overview

The Storage System is responsible for:

* Storing audio files
* Managing configuration data
* Providing file access for playback
* Supporting file upload via web UI

---

# 🧠 Design Goals

* Reliable file storage
* Low memory usage
* Fast read access for streaming
* Simple file structure
* Minimal wear on flash memory

---

# 🏗️ Storage Architecture

```text id="f0g3u2"
Web Upload → File Manager → Storage (SPIFFS / LittleFS / External Flash)
                                      ↓
                                  Audio Engine
```

---

# 📦 Storage Options

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

* SPI-based flash (8MB / 16MB)
* Useful for larger audio files

---

# 📁 File Structure

## Basic Layout

```text id="3a0v19"
/audio.wav          ← main audio file
/config.json        ← configuration file
```

---

## Rules

* Only one active audio file (current version)
* Fixed filenames for simplicity

---

# 📤 File Upload Flow

```text id="d3p8mp"
Browser → POST /upload
        ↓
Web Server
        ↓
File Manager
        ↓
Write file to storage
        ↓
EVENT_UPLOAD_DONE (optional)
```

---

# ⚙️ File Manager Module

## Responsibility

* Handle file read/write operations
* Manage storage access
* Provide streaming support

---

## Key Functions

* `saveFile()`
* `readFile()`
* `openFileStream()`
* `deleteFile()`

---

# 🔄 Audio File Handling

## Upload Behavior

* Replace existing file
* Overwrite `/audio.wav`

---

## Playback Behavior

* File opened in read mode
* Streamed in chunks

---

# ⚡ Streaming Support

## Design

* Read small chunks (512–1024 bytes)
* Pass to Audio Engine
* Repeat until EOF

---

## Benefits

* Low RAM usage
* Smooth playback

---

# 🧾 Configuration Storage

## File

```text id="j1f6pp"
/config.json
```

---

## Example Content

```json
{
  "delay": 5
}
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

* Do not load entire file into RAM
* Use streaming
* Avoid large buffers

---

# 🚫 Not Allowed

* Multiple large files (current version)
* Frequent rewriting of config
* Blocking file operations

---

# 🧪 Testing Guidelines

* Upload audio file via UI
* Verify file saved correctly
* Test playback after upload
* Verify config persistence after reboot

---

# 🔄 Error Handling

* File not found → log error
* Write failure → log error
* Corrupt file → skip playback

---

# 🚀 Future Enhancements

* Multiple audio files
* File browser UI
* Storage usage display
* External SD card support

---

# 🏁 Summary

The Storage System provides:

* Reliable file handling
* Efficient streaming support
* Persistent configuration storage
* Integration with upload and playback systems

---

END OF FILE
