# 01-overview.md

## 🎯 Project Overview

The **ESP32 Audio Greeting System in Car** is a WiFi-enabled embedded system designed to:

* Play a predefined audio file(Welcome note or Greeting audio file) automatically on power-up
* Allow users to upload audio files via a web interface
* Provide configuration controls (e.g., playback delay)
* Output audio to external stereo systems via AUX
* Offer real-time logging and monitoring via a web UI
* Secure all operations using authentication

---

## 🧠 Key Objectives

* Provide a **simple and reliable audio playback system**
* Enable **wireless configuration and control**
* Ensure **non-blocking and stable operation**
* Maintain **clean, modular, and scalable architecture**

---

## 🚀 Core Features

### 🔊 Audio Playback

* Plays audio automatically after device power ON
* Playback occurs only once per power cycle
* Audio is streamed from storage (no full RAM load)

---

### 🌐 Web-Based Control

* Embedded web UI (served directly from firmware)
* Accessible via browser over WiFi
* No external app required

---

### 📤 Audio Upload

* Upload audio files directly from browser
* Replace existing audio file
* Supports standard formats (WAV recommended)

---

### ⏱ Configurable Delay

* Set delay before playback
* Stored persistently
* Configurable via UI

---

### 📊 Logging System

* Centralized logging module
* View logs via web interface
* Supports log levels (INFO, WARN, ERROR)

---

### 📺 Log Viewer UI

* Dedicated web page for logs
* Auto-refresh capability
* Debug system without serial monitor

---

### 🔐 Authentication

* Basic Authentication (username/password)
* Protects all critical endpoints
* Prevents unauthorized access

---

## 🏗️ System Behavior

### 🔄 Boot Flow

1. Device powers ON
2. System initializes modules
3. Configuration is loaded
4. Scheduler starts delay timer
5. After delay → playback event triggered
6. Audio plays once
7. System remains idle and responsive

---

### 🌐 User Interaction Flow

1. User connects to device IP via browser
2. Authentication prompt appears
3. User logs in
4. User can:

   * Upload audio
   * Set delay
   * Trigger playback manually
   * View logs

---

## 🔌 Hardware Overview

### 🧠 Microcontroller

* ESP32 (with built-in WiFi)

---

### 💾 Storage

* Internal SPIFFS / LittleFS OR
* External SPI flash (optional)

---

### 🔊 Audio Output

* I2S interface from ESP32
* External DAC module (e.g., MAX98357A)
* Output via AUX (line-level)

---

### 🔌 Power

* 5V input with 3.3V regulation
* Stable power required for audio quality

---

## 📡 Connectivity

* WiFi-based access
* Works within local network
* No internet dependency required

---

## ⚙️ Technology Stack

* Arduino Framework (ESP32)
* FreeRTOS (built into ESP32)
* Embedded Web Server
* I2S Audio Driver

---

## 🧩 Design Philosophy

This project is designed with:

* Simplicity in operation
* Reliability in execution
* Flexibility for future expansion

---

## 🚫 Out of Scope (Current Version)

* Bluetooth audio playback
* Multi-user authentication
* Cloud integration
* HTTPS encryption

(These may be added in future versions)

---

## 🏁 Summary

This system provides a **self-contained, configurable, and secure audio playback solution** using ESP32, combining:

* Embedded systems
* Web technologies
* Event-driven architecture

---

END OF FILE
