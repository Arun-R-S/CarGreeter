# 14-build-deployment.md

## 🛠️ Build & Deployment Guide

This document explains how to:

* Set up the development environment
* Build the firmware
* Upload it to ESP32
* Test the system

---

# 🧠 Supported Environment

## Primary

* Arduino IDE (recommended for simplicity)

---

## Optional

* PlatformIO (for advanced development)

---

# ⚙️ Prerequisites

## 🧰 Software

* Arduino IDE (latest version)
* ESP32 Board Package

---

## 📦 Install ESP32 Board

### Steps:

1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add this URL in “Additional Boards Manager URLs”:

```text id="f9v0e4"
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

4. Go to **Tools → Board → Boards Manager**
5. Search: `esp32`
6. Install **ESP32 by Espressif Systems**

---

# 🔌 Hardware Setup

## Required Components

* ESP32-C3 Super Mini Development Board (HW-466AB)
* JQ6500 Voice Sound Module (16Mbit)
* Speaker or AUX output
* USB cable

---

## Example Wiring

```text id="7yo3l1"
ESP32-C3                 → JQ6500
-----------------------------------------
GND                      → GND
3V3 or 5V (per module)   → VCC
TX (GPIO of choice)      → RX
RX (GPIO of choice)      → TX (optional)

JQ6500                   → AUX Jack / Car Stereo
-----------------------------------------
DACL                     → AUX Left (via DC-blocking cap)
DACR                     → AUX Right (via DC-blocking cap)
GND                      → AUX Ground
```

---

# 📁 Project Setup

## Folder Structure (Arduino)

```text id="t4szlj"
ESP32_Audio_Player/
└── src/
    ├── ESP32_Audio_Player.ino
    ├── *.cpp
    ├── *.h
```

---

## Open Project

* Open `.ino` file in Arduino IDE
* Ensure all `.cpp/.h` files are in same folder

---

# 📚 Required Libraries

Install via Library Manager:

* WiFi
* WebServer (ESP32)
* FS (built-in)
* SPIFFS / LittleFS (built-in)

---

# ⚙️ Build Configuration

## Board Settings

* Board: **ESP32C3 Dev Module**
* Flash Size: 4MB (or higher)
* Partition Scheme: Default (enable SPIFFS only if you persist config)
* Upload Speed: 115200 or higher

---

# 🚀 Build & Upload

## Steps

1. Connect ESP32 via USB
2. Select correct COM port
3. Click **Upload**
4. Wait for compilation & flashing

---

# 📡 First Boot

## Steps

1. Open Serial Monitor (115200 baud)
2. Observe logs
3. Connect ESP32 to WiFi (if configured)

---

## Find Device IP

* Printed in Serial logs
* Example:

```text id="kq1m0c"
Connected to WiFi
IP Address: 192.168.1.10
```

---

# 🌐 Access Web UI

1. Open browser
2. Enter:

```text id="g3a7dr"
http://<ESP32_IP>
```

---

3. Enter credentials:

```text id="07chv2"
Username: admin
Password: 1234
```

---

# 🧪 Testing Steps

## ✅ Basic Tests

* Open UI page
* Set delay
* Trigger playback
* View logs

---

## 🔊 Audio Test

* Ensure DAC is connected
* Verify sound output
* Check for distortion

---

# ⚠️ Troubleshooting

## ❌ Upload Failed

* Check COM port
* Hold BOOT button while uploading (if needed)

---

## ❌ No Sound

* Check wiring
* Verify DAC power
* Ensure correct pins configured

---

## ❌ UI Not Loading

* Verify IP address
* Check WiFi connection
* Check authentication

---

## ❌ Crash / Restart

* Check Serial logs
* Look for memory issues
* Verify no blocking code

---

# 🔄 Rebuilding

* Modify code
* Click Upload again
* Device will be reflashed

---

# 📦 Production Deployment

## Recommendations

* Use stable power supply
* Secure enclosure
* Proper audio shielding

---

# 🚀 Future Deployment Options

* OTA (Over-the-Air updates)
* PlatformIO CI/CD pipeline
* Automated builds

---

# 🏁 Summary

This guide ensures:

* Easy setup and deployment
* Smooth development workflow
* Reliable testing and validation

---

END OF FILE
