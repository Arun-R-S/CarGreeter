# ESP32-C3 Car Greeter (JQ6500 Audio)

A **WiFi-enabled, event-driven audio greeting system in car** built on **ESP32-C3 + JQ6500**, that plays a personalized welcome message on car startup, configurable via 🌐 web UI with ⏱ delay control.

---

## 🚀 Features

* 🔊 Automatic audio playback on power-up
* 🌐 Web-based control (no app required)
* ⏱ Configurable playback delay
* 📊 Real-time log viewer UI
* 🔐 Basic Authentication for security
* ⚡ Non-blocking, event-driven architecture

---

## 🧠 Project Overview

This project uses an **ESP32-C3 microcontroller** to:

* Control a JQ6500 module that stores the welcome audio in its internal flash
* Play it automatically after boot (once per power cycle)
* Allow configuration and control via a web interface

For a detailed overview and complete documentation map, see **[📘 Full Project Overview](docs/01-overview.md)**.

---

## 🏗️ Architecture Highlights

* ✅ **Modular design**
* ✅ **Event-driven communication**
* ✅ **FreeRTOS-based tasks**
* ✅ **Non-blocking execution**

---

* ESP32-C3 or ESP32-Cam
* JQ6500 Voice Sound Module (16Mbit)
* External speaker / AUX output
* JQ6500 internal flash storage for the welcome track

---

## 🔌 Hardware Wiring (ESP32-Cam)

| ESP32-Cam Pin | JQ6500 Pin | Function |
| :--- | :--- | :--- |
| **5V** | **VCC** | Power (5V recommended) |
| **GND** | **GND** | Common Ground |
| **GPIO 13** | **RX** | UART Control Signal |


---

## 🌐 Web Interface

Access the device using a browser:

```text
http://<ESP32_IP>
```

### Features:

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
│   ├── jq6500_player.cpp
│   ├── jq6500_player.h
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
* Program the welcome audio onto the JQ6500 module (per its documentation)
* Set delay
* Reboot device to test auto-play

---

## ⚠️ Notes

* Use only on trusted local networks
* Audio is stored/decoded on the JQ6500 module (ESP32-C3 only controls playback)

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
