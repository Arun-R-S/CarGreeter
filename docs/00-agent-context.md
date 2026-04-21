# 00-agent-context.md

This file is a **quick, repo-local memory** for humans/agents working on `CarGreeter`.
It is a *summary* of `AGENTS.md` and `docs/*.md` (the docs remain the source of truth).

## Non-negotiables (from `AGENTS.md` + `docs/12-non-functional.md`)

- **No `delay()`** anywhere in the firmware.
- **No direct module-to-module calls** (ex: web -> audio). Communication is **Event Bus only**.
- **No business logic in web handlers**. Web handlers validate/auth + translate request -> event + response.
- **No direct Serial logging in modules**. Use Logger API (ex: `logInfo("AUDIO", "...")`).
- Avoid large dynamic allocations; keep buffers small; keep handlers fast/non-blocking.

## High-level architecture (from `docs/02-architecture.md`)

`Web UI (browser)` -> `Web Server` -> `Auth Manager` -> `Event Bus` -> `Modules` (Audio/Scheduler/Config/Network/System/Time)  
Logger is queue-based and is used by all modules.

FreeRTOS task model (conceptually):
- Web Server task
- Event Bus dispatch
- JQ6500 audio task
- Scheduler task
- Logger task
- Config/Network/System/Time tasks (as needed)

## Modules (from `docs/03-modules.md`)

- **Web Server**: routes + auth check + emit events; no business logic.
- **Auth Manager**: Basic Auth guard; credentials loaded from Config Manager (defaults `admin` / `1234`).
- **Event Bus**: FreeRTOS queue; `Event { type, value(optional) }`.
- **JQ6500 Player**: consumes play/stop/volume events; emits UART commands only.
- **Config Manager (Settings)**: loads defaults/CRC/versioned settings; debounced persistence; backup/restore.
- **Scheduler**: boot delay; triggers one-time `EVENT_PLAY` per power cycle.
- **Logger**: non-blocking log queue; Serial + web circular buffer (25).
- **Network Manager**: WiFi state management; scan/connect/forget/hotspot via events.
- **System Manager**: `/api/sysinfo` metrics (heap/flash/uptime/tasks/network summary).
- **Time Manager**: timestamps + browser/NTP sync events.

## Event types (from `docs/04-event-system.md`)

Core:
- `EVENT_PLAY` = 0
- `EVENT_SET_DELAY` = 1
- `EVENT_STOP` = 2
- `EVENT_VOLUME_CHANGE` = 3
- `EVENT_ERROR` = 4

WiFi / Hotspot:
- `EVENT_WIFI_SET` = 5
- `EVENT_WIFI_FORGET` = 6
- `EVENT_HOTSPOT_SET` = 7
- `EVENT_WIFI_SCAN` = 12

Audio config:
- `EVENT_AUDIO_SET_PRELOADED` = 8
- `EVENT_AUDIO_SET_CUSTOM` = 9

System:
- `EVENT_SYSTEM_RESTART` = 10
- `EVENT_SYSTEM_FACTORY_RESET` = 11

Time:
- `EVENT_TIME_SYNC` = 13
- `EVENT_TIME_SYNC_NTP` = 14
- `EVENT_AUTH_SET` = 15

## HTTP API + UI pages (from `docs/11-api-spec.md`, `docs/05-web-ui.md`, `docs/06-authentication.md`)

All endpoints are **Basic-Auth protected**.

UI pages:
- `GET /` (main UI)
- `GET /logview` (log viewer UI)
- `GET /sysinfo` (system info UI)

Core control:
- `GET /play`
- `GET /setDelay?value=<int>`
- `GET /logs` (recent 25 as plain text)

Settings:
- `GET /api/settings` (JSON)
- `GET /api/sysinfo` (JSON)
- `GET /api/backup` (plain text key=value)
- `POST /api/restore` (plain text key=value via multipart or raw)
- `POST /api/auth` (username/password; restarts)

WiFi:
- `GET /api/wifi/scan`
- `POST /api/wifi/connect` (`ssid`, `password`)
- `POST /api/wifi/forget`
- `POST /api/hotspot` (`ssid`, `password`)

Audio:
- `POST /api/audio` (`volume` 0-100, `preloadedIndex`, `customIndex`)

Maintenance:
- `POST /api/restart`
- `POST /api/factory_reset`
- `POST /api/time/sync` (`epochMsUtc`)

Web UI storage:
- Embedded HTML/JS lives in `web_pages.h` using `PROGMEM`.

## Storage + backup/restore (from `docs/09-storage.md`, `docs/11-api-spec.md`)

- Primary persistent store: **NVS/Preferences**
  - namespace: `car_greeter`
  - keys: `cfgA` / `cfgB` (A/B slots)
- Writes are **deferred/debounced** and should be triggered by events (not inside handlers).
- Backup file includes secrets (WiFi/AP/admin creds) and is **plain text**:
  - `delaySeconds`, `volume`, `preloadedIndex`, `customIndex`
  - `wifiSsid`, `wifiPassword`
  - `apSsid`, `apPassword`
  - `adminUsername`, `adminPassword`

## Audio model (from `docs/07-audio-system.md`)

- ESP32-C3 does **not** decode/stream audio.
- JQ6500 stores audio in its internal flash; ESP32 controls via UART.
- Outputs are analog via JQ6500 `DACL/DACR` to car AUX.

## Quick “done right” checklist

- Web handler: `auth -> validate params -> sendEvent(...) -> return response`.
- Module action: `wait for event -> act -> log`.
- No blocking waits; use `millis()` or short `vTaskDelay()` inside tasks only.
- Add new features by **adding new events and/or new modules**, then document updates in `/docs`.

