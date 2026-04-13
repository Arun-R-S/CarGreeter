#include "config_manager.h"

#include <Preferences.h>
#include <stddef.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "event_bus.h"
#include "logger.h"

namespace {

constexpr char kPrefsNamespace[] = "car_greeter";
constexpr char kKeySlotA[] = "cfgA";
constexpr char kKeySlotB[] = "cfgB";

constexpr uint32_t kMagic = 0x43524754u;  // 'CRGT'
constexpr uint16_t kVersion = 2;

constexpr int32_t kDefaultDelaySeconds = 5;
constexpr int32_t kDefaultVolume = 20;
constexpr uint16_t kDefaultWelcomeTrackIndex = 1;

constexpr size_t kWifiSsidMaxLen = 32;
constexpr size_t kWifiPasswordMaxLen = 64;

constexpr uint32_t kDebounceMs = 3000u;
constexpr uint32_t kMinWriteIntervalMs = 30000u;

struct SettingsV2 {
  int32_t delaySeconds;
  int32_t volume;
  uint16_t welcomeTrackIndex;
  uint16_t reserved;
  char wifiSsid[kWifiSsidMaxLen + 1];
  char wifiPassword[kWifiPasswordMaxLen + 1];
};

struct SettingsRecordV2 {
  uint32_t magic;
  uint16_t version;
  uint16_t size;
  uint32_t seq;
  SettingsV2 settings;
  uint32_t crc32;
};

TaskHandle_t g_task = nullptr;

// NOTE: Keep initialization compatible with the Arduino ESP32 toolchain (C++11).
SettingsV2 g_settings = {
    kDefaultDelaySeconds,
    kDefaultVolume,
    kDefaultWelcomeTrackIndex,
    0,
    {0},
    {0},
};
uint32_t g_seq = 0;
bool g_lastSlotWasA = true;

volatile bool g_dirty = false;
volatile uint32_t g_lastChangeMs = 0;
volatile uint32_t g_lastSaveMs = 0;

uint32_t crc32(const uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; i++) {
    crc ^= static_cast<uint32_t>(data[i]);
    for (int b = 0; b < 8; b++) {
      const uint32_t mask = (crc & 1u) ? 0xFFFFFFFFu : 0u;
      crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
  }
  return ~crc;
}

SettingsV2 clampSettings(SettingsV2 s) {
  if (s.delaySeconds < 0) {
    s.delaySeconds = 0;
  } else if (s.delaySeconds > 3600) {
    s.delaySeconds = 3600;
  }

  if (s.volume < 0) {
    s.volume = 0;
  } else if (s.volume > 30) {
    s.volume = 30;
  }

  if (s.welcomeTrackIndex == 0) {
    s.welcomeTrackIndex = kDefaultWelcomeTrackIndex;
  }
  s.wifiSsid[kWifiSsidMaxLen] = '\0';
  s.wifiPassword[kWifiPasswordMaxLen] = '\0';
  return s;
}

bool loadSlot(Preferences& prefs, const char* key, SettingsRecordV2* out) {
  if (out == nullptr || key == nullptr) {
    return false;
  }
  const size_t len = prefs.getBytesLength(key);
  if (len != sizeof(SettingsRecordV2)) {
    return false;
  }
  const size_t n = prefs.getBytes(key, out, sizeof(SettingsRecordV2));
  if (n != sizeof(SettingsRecordV2)) {
    return false;
  }
  if (out->magic != kMagic || out->version != kVersion || out->size != sizeof(SettingsV2)) {
    return false;
  }
  const uint32_t expected = crc32(reinterpret_cast<const uint8_t*>(out), offsetof(SettingsRecordV2, crc32));
  if (expected != out->crc32) {
    return false;
  }
  out->settings = clampSettings(out->settings);
  return true;
}

bool loadBestFromNvs(SettingsV2* outSettings, uint32_t* outSeq, bool* outLastSlotWasA) {
  if (outSettings == nullptr || outSeq == nullptr || outLastSlotWasA == nullptr) {
    return false;
  }

  Preferences prefs;
  // IMPORTANT:
  // - `Preferences.begin(ns, true)` uses NVS_READONLY and fails with NOT_FOUND
  //   on a fresh device (namespace not created yet).
  // - Open read-write even for reads so we can detect "no keys" vs. hard failure.
  if (!prefs.begin(kPrefsNamespace, false)) {
    logWarn("CONF", "NVS begin failed");
    return false;
  }

  SettingsRecordV2 recA{};
  SettingsRecordV2 recB{};
  const bool okA = loadSlot(prefs, kKeySlotA, &recA);
  const bool okB = loadSlot(prefs, kKeySlotB, &recB);
  prefs.end();

  if (!okA && !okB) {
    return false;
  }

  if (okA && (!okB || recA.seq >= recB.seq)) {
    *outSettings = recA.settings;
    *outSeq = recA.seq;
    *outLastSlotWasA = true;
    return true;
  }

  *outSettings = recB.settings;
  *outSeq = recB.seq;
  *outLastSlotWasA = false;
  return true;
}

bool saveToNvsSlot(const SettingsV2& settings, uint32_t seq, bool slotA) {
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, false)) {
    logWarn("CONF", "NVS begin failed");
    return false;
  }

  SettingsRecordV2 rec{};
  rec.magic = kMagic;
  rec.version = kVersion;
  rec.size = sizeof(SettingsV2);
  rec.seq = seq;
  rec.settings = clampSettings(settings);
  rec.crc32 = crc32(reinterpret_cast<const uint8_t*>(&rec), offsetof(SettingsRecordV2, crc32));

  const char* key = slotA ? kKeySlotA : kKeySlotB;
  const size_t written = prefs.putBytes(key, &rec, sizeof(SettingsRecordV2));
  prefs.end();
  return written == sizeof(SettingsRecordV2);
}

void scheduleSave() {
  g_dirty = true;
  g_lastChangeMs = millis();
  if (g_task != nullptr) {
    xTaskNotifyGive(g_task);
  }
}

void onSetDelay(const Event& event, void*) {
  int32_t v = event.value;
  if (v < 0) {
    v = 0;
  }
  if (v > 3600) {
    v = 3600;
  }
  if (v == g_settings.delaySeconds) {
    return;
  }
  g_settings.delaySeconds = v;
  scheduleSave();
  logInfo("CONF", "Delay updated");
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "Delay updated to %ld seconds", v);
  logDebug("CONF", buffer);
}

void configTask(void*) {
  for (;;) {
    (void)ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500));

    if (!g_dirty) {
      continue;
    }

    const uint32_t now = millis();
    if ((now - g_lastChangeMs) < kDebounceMs) {
      continue;
    }
    if (g_lastSaveMs != 0 && (now - g_lastSaveMs) < kMinWriteIntervalMs) {
      continue;
    }

    const bool nextSlotA = !g_lastSlotWasA;
    const uint32_t nextSeq = g_seq + 1;
    const bool ok = saveToNvsSlot(g_settings, nextSeq, nextSlotA);
    g_lastSaveMs = now;
    if (!ok) {
      logWarn("CONF", "Settings save failed");
      continue;
    }

    g_seq = nextSeq;
    g_lastSlotWasA = nextSlotA;
    g_dirty = false;
    logInfo("CONF", "Settings saved");
  }
}

}  // namespace

void configManagerInit() {
  (void)eventBusRegisterHandler(EVENT_SET_DELAY, onSetDelay, nullptr);

  SettingsV2 loaded{};
  uint32_t loadedSeq = 0;
  bool lastWasA = true;
  if (loadBestFromNvs(&loaded, &loadedSeq, &lastWasA)) {
    g_settings = loaded;
    g_seq = loadedSeq;
    g_lastSlotWasA = lastWasA;
    logInfo("CONF", "Settings loaded");
  } else {
    g_settings = clampSettings(g_settings);
    g_seq = 0;
    g_lastSlotWasA = true;
    logWarn("CONF", "No valid settings; using defaults");
    // Persist defaults once so subsequent boots have a valid settings record.
    scheduleSave();
  }

  (void)eventBusSend(EVENT_SET_DELAY, g_settings.delaySeconds);
}

void configManagerStartTask(UBaseType_t priority, uint32_t stackWords) {
  if (g_task != nullptr) {
    return;
  }
  const BaseType_t ok = xTaskCreate(configTask, "settings", stackWords, nullptr, priority, &g_task);
  if (ok != pdPASS) {
    g_task = nullptr;
    logError("CONF", "Failed to start settings task");
  }
}

bool configManagerHasWifiCredentials() {
  return g_settings.wifiSsid[0] != '\0' && g_settings.wifiPassword[0] != '\0';
}

void configManagerCopyWifiCredentials(char* ssidOut,
                                      size_t ssidOutSize,
                                      char* passwordOut,
                                      size_t passwordOutSize) {
  if (ssidOut != nullptr && ssidOutSize > 0) {
    snprintf(ssidOut, ssidOutSize, "%s", g_settings.wifiSsid);
  }
  if (passwordOut != nullptr && passwordOutSize > 0) {
    snprintf(passwordOut, passwordOutSize, "%s", g_settings.wifiPassword);
  }
}
