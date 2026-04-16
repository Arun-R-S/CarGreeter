#include "jq6500_player.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "../core/event_bus.h"
#include "../core/logger.h"

namespace {

TaskHandle_t g_task = nullptr;
HardwareSerial* g_serial = &Serial2;
Jq6500Config g_cfg{.txPin = -1, .rxPin = -1, .baudRate = 9600, .welcomeTrackIndex = 1, .volume = 20};

volatile bool g_playRequested = false;
volatile uint16_t g_preloadedTrackIndex = 1;
volatile uint16_t g_customTrackIndex = 0;
volatile uint8_t g_volume = 20;

void writeBytes(const uint8_t* data, size_t len) {
  if (data == nullptr || len == 0) {
    return;
  }
  g_serial->write(data, len);
  g_serial->flush();
}

// JQ6500 Serial Protocol (common on "Voice Module" boards):
// - No-param: [0x7E, 0x02, CMD, 0xEF]
// - 1 param byte: [0x7E, 0x03, CMD, PARAM, 0xEF]
// - 2 param bytes: [0x7E, 0x04, CMD, PARAM_H, PARAM_L, 0xEF]
void sendNoParam(uint8_t cmd) {
  const uint8_t pkt[] = {0x7E, 0x02, cmd, 0xEF};
  writeBytes(pkt, sizeof(pkt));
}

void send1(uint8_t cmd, uint8_t param) {
  const uint8_t pkt[] = {0x7E, 0x03, cmd, param, 0xEF};
  writeBytes(pkt, sizeof(pkt));
}

void send2(uint8_t cmd, uint16_t param) {
  const uint8_t hi = static_cast<uint8_t>((param >> 8) & 0xFF);
  const uint8_t lo = static_cast<uint8_t>(param & 0xFF);
  const uint8_t pkt[] = {0x7E, 0x04, cmd, hi, lo, 0xEF};
  writeBytes(pkt, sizeof(pkt));
}

void setDeviceFlash() {
  // CMD 0x09: assigned devices; FLASH is commonly 0x04 on many boards.
  send1(0x09, 0x04);
}

void setVolume(uint8_t volume) {
  if (volume > 30) {
    volume = 30;
  }
  g_volume = volume;
  send1(0x06, volume);
}

void playTrack(uint16_t index) { send2(0x03, index); }

void onPlay(const Event&, void*) { g_playRequested = true; }

void onSetPreloaded(const Event& event, void*) {
  int32_t v = event.value;
  if (v < 1) {
    v = 1;
  }
  if (v > 9999) {
    v = 9999;
  }
  g_preloadedTrackIndex = static_cast<uint16_t>(v);
}

void onSetCustom(const Event& event, void*) {
  int32_t v = event.value;
  if (v < 0) {
    v = 0;
  }
  if (v > 9999) {
    v = 9999;
  }
  g_customTrackIndex = static_cast<uint16_t>(v);
}

void onVolume(const Event& event, void*) {
  int32_t v = event.value;
  if (v < 0) {
    v = 0;
  }
  if (v > 30) {
    v = 30;
  }
  setVolume(static_cast<uint8_t>(v));
}

void jqTask(void*) {
  setDeviceFlash();
  setVolume(static_cast<uint8_t>(g_cfg.volume));
  logInfo("JQ6500", "Ready");

  for (;;) {
    if (!g_playRequested) {
      vTaskDelay(pdMS_TO_TICKS(25));
      continue;
    }
    g_playRequested = false;

    logInfo("JQ6500", "Play requested");
    const uint16_t idx = (g_customTrackIndex != 0) ? g_customTrackIndex : g_preloadedTrackIndex;
    playTrack(idx);
  }
}

}  // namespace

void jq6500PlayerInit(const Jq6500Config& config) {
  g_cfg = config;
  if (g_cfg.baudRate == 0) {
    g_cfg.baudRate = 9600;
  }
  if (g_cfg.welcomeTrackIndex == 0) {
    g_cfg.welcomeTrackIndex = 1;
  }
  g_preloadedTrackIndex = g_cfg.welcomeTrackIndex;
  g_customTrackIndex = 0;
  g_volume = static_cast<uint8_t>(g_cfg.volume);

  if (g_cfg.txPin == 16 || g_cfg.txPin == 17) {
    logWarn("JQ6500", "TX pin 16/17 may conflict with PSRAM on some ESP32 modules");
  }
  if (g_cfg.txPin == 9 || g_cfg.txPin == 10) {
    logWarn("JQ6500", "TX pin 9/10 may conflict with SPI flash on some ESP32 modules");
  }
  if (g_cfg.txPin == 4) {
    logWarn("JQ6500", "TX pin 4 may drive the ESP32-CAM flash LED on some boards");
  }

  // IMPORTANT (classic ESP32/ESP32-CAM):
  // - UART1 defaults to GPIO9/10 which are tied to SPI flash.
  // - Use UART2 (Serial2) with explicit pins, and allow TX-only wiring if desired.
  if (g_cfg.txPin < 0) {
    logError("JQ6500", "TX pin not set; JQ6500 disabled");
    return;
  }
  g_serial->begin(g_cfg.baudRate, SERIAL_8N1, g_cfg.rxPin, g_cfg.txPin);

  (void)eventBusRegisterHandler(EVENT_PLAY, onPlay, nullptr);
  (void)eventBusRegisterHandler(EVENT_AUDIO_SET_PRELOADED, onSetPreloaded, nullptr);
  (void)eventBusRegisterHandler(EVENT_AUDIO_SET_CUSTOM, onSetCustom, nullptr);
  (void)eventBusRegisterHandler(EVENT_VOLUME_CHANGE, onVolume, nullptr);
}

void jq6500PlayerStartTask(UBaseType_t priority, uint32_t stackWords) {
  if (g_task != nullptr) {
    return;
  }
  const BaseType_t ok = xTaskCreate(jqTask, "jq6500", stackWords, nullptr, priority, &g_task);
  if (ok != pdPASS) {
    g_task = nullptr;
    logError("JQ6500", "Failed to start task");
  }
}
