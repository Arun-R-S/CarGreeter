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
| **5V** | **VCC** | Power (5V) |
| **GND** | **GND** | Common Ground |
| **GPIO 13** | **RX** | UART Control Signal |
| **N/A** | **SPK+ / SPK-** | Direct Speaker Output (Onboard Amp) |
| **N/A** | **ADL / ADR** | AUX / Line Out (Audio DAC L/R) |



---

## 🌐 Web Interface

The system features a modern, responsive web dashboard (Tasmota-style) accessible via any browser at `http://<ESP32_IP>`.

### Key Features:

*   **🔊 Audio Control:**
    *   Set **Volume level** (0–30).
    *   Configure **Playback Delay** (0–3600 seconds).
    *   Select from **50 Preloaded tracks** or specify a **Custom Index**.
    *   **Direct Play:** Instant preview button (▶) to test any track index without saving.
    *   **Dynamic Title:** Displays the currently active track index in real-time.
*   **📶 Network Management:**
    *   **WiFi Scan & Connect:** Visual list of nearby networks with RSSI (signal) strength.
    *   **Hotspot Control:** Change AP name and password.
    *   **IP Monitoring:** View connection status and IP address in the header.
*   **🔐 Security & Admin:**
    *   Change **Admin credentials** for Basic Authentication.
    *   **Backup & Restore:** Download your entire configuration as a text file and restore it to a new device instantly.
*   **📊 Monitoring & Maintenance:**
    *   **Live Logs:** Real-time log viewer for troubleshooting UART and Network events.
    *   **Advanced SysInfo:** Detailed dashboard showing Heap memory, CPU task list, and Flash usage.
    *   **Remote Actions:** Restart or Factory Reset the device via the UI.

---

## 🔐 Default Credentials

```text
Username: admin
Password: 1234
```

---

## 📁 Project Structure

The project follows a clean, modular architecture separating generic core services from application-specific logic.

```text
CarGreeter/
│
├── /src/CarGreeter
│   ├── CarGreeter.ino        ← Main entry point
│   └── /src
│       ├── /app             ← Application Logic
│       │   ├── config_manager  ← NVS settings & defaults
│       │   ├── jq6500_player   ← Audio driver & timing safety
│       │   ├── scheduler      ← Boot delay & playback triggers
│       │   └── web_server     ← API endpoints & Web UI
│       │
│       ├── /core            ← Infrastructure (Reusable)
│       │   ├── event_bus      ← Non-blocking message system
│       │   ├── auth_manager   ← Basic Authentication
│       │   ├── network_mgr    ← WiFi & AP management
│       │   ├── system_mgr     ← Task & RAM monitoring
│       │   └── logger        ← Circular log buffer
│       │
│       └── build_config.h    ← ⭐ HARDWARE & BRANDING DEFAULTS
│
├── /docs                    ← Detailed design specs
└── AGENT.md                 ← AI Agent instructions
```

---

## ⚙️ How It Works (Event Flow)

1.  **⚡ Power On:** The system initializes the `Event Bus` and `Logger` immediately.
2.  **💾 Config Load:** `Config Manager` loads saved volume and delay from NVS. If empty, it uses `build_config.h` defaults.
3.  **🔊 Audio Prep:** `JQ6500 Player` starts and waits **500ms** for the hardware to stabilize, then syncs the volume.
4.  **⏱️ Greeting Delay:** The `Scheduler` waits for the configured delay (e.g., 5s). This allows the car's engine to start and electronics to stabilize.
5.  **🎶 Playback:** After the delay, `Scheduler` sends `EVENT_PLAY`. The Player receives this, re-syncs volume, waits **100ms**, and triggers the JQ6500.
6.  **🌐 Ready Mode:** The Web Server remains active, allowing you to change settings, view logs, or play tracks manually via the UI.

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
