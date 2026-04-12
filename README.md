# ESP32 Car Greeter 🚗🔊

A **WiFi-enabled, event-driven audio greeting system in car** built on ESP32, that plays a personalized welcome message on car startup, configurable via 🌐 web UI with ⏱ delay control and 📤 audio upload.

---

## 🚀 Features

* 🔊 Automatic audio playback on power-up
* 🌐 Web-based control (no app required)
* 📤 Upload audio files via browser
* ⏱ Configurable playback delay
* 📊 Real-time log viewer UI
* 🔐 Basic Authentication for security
* ⚡ Non-blocking, event-driven architecture

---

## 🧠 Project Overview

This project uses an **ESP32 microcontroller** to:

* Store an audio file in flash memory
* Play it automatically after boot (once per power cycle)
* Allow configuration and control via a web interface

---

## 🏗️ Architecture Highlights

* ✅ **Modular design**
* ✅ **Event-driven communication**
* ✅ **FreeRTOS-based tasks**
* ✅ **Non-blocking execution**

---

## 📦 System Components

* ESP32 (WiFi-enabled microcontroller)
* I2S DAC (e.g., MAX98357A)
* External speaker / AUX output
* Internal or external flash storage

---

## 🌐 Web Interface

Access the device using a browser:

```text
http://<ESP32_IP>
```

### Features:

* Upload audio file
* Set playback delay
* Trigger playback manually
* View system logs

---

## 🔐 Default Credentials

```text
Username: admin
Password: 1234
```

---

## 📁 Project Structure

```text
CarGreeter/
│
├── AGENT.md                 ← ⭐ PRIMARY FILE (entry point for agent)
├── README.md                ← Human-friendly overview
│
├── /docs                    ← All detailed specifications
│   │
│   ├── 01-overview.md
│   ├── 02-architecture.md
│   ├── 03-modules.md
│   ├── 04-event-system.md
│   ├── 05-web-ui.md
│   ├── 06-authentication.md
│   ├── 07-audio-system.md
│   ├── 08-logging.md
│   ├── 09-storage.md
│   ├── 10-scheduler.md
│   ├── 11-api-spec.md
│   ├── 12-non-functional.md
│   ├── 13-future-enhancements.md
│   ├── 14-build-deployment.md
│
├── /src                     ← Arduino source files
│   │
│   ├── CarGreeter.ino
│   │
│   ├── web_server.cpp
│   ├── web_server.h
│   │
│   ├── web_pages.h          ← Embedded UI (Tasmota-style)
│   │
│   ├── auth_manager.cpp
│   ├── auth_manager.h
│   │
│   ├── event_bus.cpp
│   ├── event_bus.h
│   │
│   ├── audio_engine.cpp
│   ├── audio_engine.h
│   │
│   ├── file_manager.cpp
│   ├── file_manager.h
│   │
│   ├── config_manager.cpp
│   ├── config_manager.h
│   │
│   ├── scheduler.cpp
│   ├── scheduler.h
│   │
│   ├── logger.cpp
│   ├── logger.h
│
└── .gitignore
```

---

## ⚙️ How It Works

1. Power ON the device
2. System initializes modules
3. Delay timer starts
4. Audio plays once after delay
5. System remains ready for user interaction

---

## 🔧 Setup & Installation

Refer to:

👉 `docs/14-build-deployment.md`

---

## 📚 Documentation

Detailed documentation is available in `/docs`:

* Architecture
* Modules
* Event system
* API specification
* Storage & audio system
* Logging & scheduler

---

## 🧪 Usage

* Open web UI
* Upload audio file (`.wav` recommended)
* Set delay
* Reboot device to test auto-play

---

## ⚠️ Notes

* Use only on trusted local networks
* WAV format recommended (no decoding required)
* Avoid large files due to memory constraints

---

## 🚀 Future Enhancements

* Multiple audio files
* Volume control
* OTA updates
* Bluetooth audio
* Advanced scheduling

---

## 🤝 Contribution

* Follow modular architecture
* Use event-driven communication
* Avoid blocking code (`delay()` not allowed)
* Update documentation for changes

---

## 📜 License

Open-source (choose your preferred license)

---

## 🏁 Summary

This project is a **lightweight, reliable, and extensible ESP32 audio greeting system for car**, combining:

* Embedded systems
* Web technologies
* Clean software architecture

---

## 👥 Contributors ✨

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/Arun-R-S">
        <img src="https://github.com/Arun-R-S.png" width="100" style="border-radius:50%; display:block;" alt="ArunRS"/><br />
        <b>@Arun-R-S</b>
      </a><br />
      💼 Owner & Developer
    </td>
    <td align="center">
      <a href="https://github.com/EchoWaveSystems">
        <img src="https://github.com/EchoWaveSystems.png" width="100" style="border-radius:50%; display:block;" alt="EchoWaveSystems"/><br />
        <b>@EchoWaveSystems</b>
      </a><br />
      💻 Developer & Reviewer
    </td>
  </tr>
</table>

---
END OF FILE
