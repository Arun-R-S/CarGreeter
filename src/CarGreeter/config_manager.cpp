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
constexpr uint16_t kVersion = 3;

constexpr int32_t kDefaultDelaySeconds = 5;
constexpr int32_t kDefaultVolume = 20;
constexpr uint16_t kDefaultWelcomeTrackIndex = 1;

constexpr size_t kWifiSsidMaxLen = 32;
constexpr size_t kWifiPasswordMaxLen = 64;

constexpr size_t kApSsidMaxLen = 32;
constexpr size_t kApPasswordMaxLen = 64;

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

struct SettingsV3 {
  int32_t delaySeconds;
  int32_t volume;
  uint16_t welcomeTrackIndex;
  uint16_t reserved;
  char wifiSsid[kWifiSsidMaxLen + 1];
  char wifiPassword[kWifiPasswordMaxLen + 1];
  char hotspotSsid[kApSsidMaxLen + 1];
  char hotspotPassword[kApPasswordMaxLen + 1];
};

struct SettingsRecordV3 {
  uint32_t magic;
  uint16_t version;
  uint16_t size;
  uint32_t seq;
  SettingsV3 settings;
  uint32_t crc32;
};

TaskHandle_t g_task = nullptr;

// NOTE: Keep initialization compatible with the Arduino ESP32 toolchain (C++11).
SettingsV3 g_settings = {
    kDefaultDelaySeconds,
    kDefaultVolume,
    kDefaultWelcomeTrackIndex,
    0,
    {0},
    {0},
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

SettingsV3 clampSettings(SettingsV3 s) {
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
  s.hotspotSsid[kApSsidMaxLen] = '\0';
  s.hotspotPassword[kApPasswordMaxLen] = '\0';
  return s;
}

bool loadSlotV3(Preferences& prefs, const char* key, SettingsRecordV3* out) {
  if (out == nullptr || key == nullptr) {
    return false;
  }
  const size_t len = prefs.getBytesLength(key);
  if (len != sizeof(SettingsRecordV3)) {
    return false;
  }
  const size_t n = prefs.getBytes(key, out, sizeof(SettingsRecordV3));
  if (n != sizeof(SettingsRecordV3)) {
    return false;
  }
  if (out->magic != kMagic || out->version != 3 || out->size != sizeof(SettingsV3)) {
    return false;
  }
  const uint32_t expected = crc32(reinterpret_cast<const uint8_t*>(out), offsetof(SettingsRecordV3, crc32));
  if (expected != out->crc32) {
    return false;
  }
  out->settings = clampSettings(out->settings);
  return true;
}

bool loadSlotV2(Preferences& prefs, const char* key, SettingsRecordV2* out) {
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
  if (out->magic != kMagic || out->version != 2 || out->size != sizeof(SettingsV2)) {
    return false;
  }
  const uint32_t expected = crc32(reinterpret_cast<const uint8_t*>(out), offsetof(SettingsRecordV2, crc32));
  if (expected != out->crc32) {
    return false;
  }
  // Clamp V2 fields that exist.
  if (out->settings.delaySeconds < 0) {
    out->settings.delaySeconds = 0;
  } else if (out->settings.delaySeconds > 3600) {
    out->settings.delaySeconds = 3600;
  }
  if (out->settings.volume < 0) {
    out->settings.volume = 0;
  } else if (out->settings.volume > 30) {
    out->settings.volume = 30;
  }
  if (out->settings.welcomeTrackIndex == 0) {
    out->settings.welcomeTrackIndex = kDefaultWelcomeTrackIndex;
  }
  out->settings.wifiSsid[kWifiSsidMaxLen] = '\0';
  out->settings.wifiPassword[kWifiPasswordMaxLen] = '\0';
  return true;
}

SettingsV3 migrateFromV2(const SettingsV2& v2) {
  SettingsV3 v3{};
  v3.delaySeconds = v2.delaySeconds;
  v3.volume = v2.volume;
  v3.welcomeTrackIndex = v2.welcomeTrackIndex;
  v3.reserved = 0;
  snprintf(v3.wifiSsid, sizeof(v3.wifiSsid), "%s", v2.wifiSsid);
  snprintf(v3.wifiPassword, sizeof(v3.wifiPassword), "%s", v2.wifiPassword);
  v3.hotspotSsid[0] = '\0';
  v3.hotspotPassword[0] = '\0';
  return clampSettings(v3);
}

bool loadBestFromNvs(SettingsV3* outSettings, uint32_t* outSeq, bool* outLastSlotWasA) {
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

  SettingsRecordV3 recA3{};
  SettingsRecordV3 recB3{};
  SettingsRecordV2 recA2{};
  SettingsRecordV2 recB2{};

  const bool okA3 = loadSlotV3(prefs, kKeySlotA, &recA3);
  const bool okB3 = loadSlotV3(prefs, kKeySlotB, &recB3);

  const bool okA = okA3 ? true : loadSlotV2(prefs, kKeySlotA, &recA2);
  const bool okB = okB3 ? true : loadSlotV2(prefs, kKeySlotB, &recB2);
  prefs.end();

  if (!okA && !okB) {
    return false;
  }

  const uint32_t seqA = okA3 ? recA3.seq : recA2.seq;
  const uint32_t seqB = okB3 ? recB3.seq : recB2.seq;

  if (okA && (!okB || seqA >= seqB)) {
    *outSettings = okA3 ? recA3.settings : migrateFromV2(recA2.settings);
    *outSeq = seqA;
    *outLastSlotWasA = true;
    return true;
  }

  *outSettings = okB3 ? recB3.settings : migrateFromV2(recB2.settings);
  *outSeq = seqB;
  *outLastSlotWasA = false;
  return true;
}

bool saveToNvsSlot(const SettingsV3& settings, uint32_t seq, bool slotA) {
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, false)) {
    logWarn("CONF", "NVS begin failed");
    return false;
  }

  SettingsRecordV3 rec{};
  rec.magic = kMagic;
  rec.version = kVersion;
  rec.size = sizeof(SettingsV3);
  rec.seq = seq;
  rec.settings = clampSettings(settings);
  rec.crc32 = crc32(reinterpret_cast<const uint8_t*>(&rec), offsetof(SettingsRecordV3, crc32));

  const char* key = slotA ? kKeySlotA : kKeySlotB;
  const size_t written = prefs.putBytes(key, &rec, sizeof(SettingsRecordV3));
  prefs.end();
  return written == sizeof(SettingsRecordV3);
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

  SettingsV3 loaded{};
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

bool configManagerHasHotspotCredentials() {
  return g_settings.hotspotSsid[0] != '\0' && g_settings.hotspotPassword[0] != '\0';
}

void configManagerCopyHotspotCredentials(char* ssidOut,
                                         size_t ssidOutSize,
                                         char* passwordOut,
                                         size_t passwordOutSize) {
  if (ssidOut != nullptr && ssidOutSize > 0) {
    snprintf(ssidOut, ssidOutSize, "%s", g_settings.hotspotSsid);
  }
  if (passwordOut != nullptr && passwordOutSize > 0) {
    snprintf(passwordOut, passwordOutSize, "%s", g_settings.hotspotPassword);
  }
}

void configManagerEnsureHotspotCredentials(const char* defaultSsid, const char* defaultPassword) {
  if (configManagerHasHotspotCredentials()) {
    return;
  }
  if (defaultSsid == nullptr || defaultPassword == nullptr || defaultSsid[0] == '\0' || defaultPassword[0] == '\0') {
    logWarn("CONF", "Default hotspot credentials missing");
    return;
  }
  snprintf(g_settings.hotspotSsid, sizeof(g_settings.hotspotSsid), "%s", defaultSsid);
  snprintf(g_settings.hotspotPassword, sizeof(g_settings.hotspotPassword), "%s", defaultPassword);
  scheduleSave();
  logInfo("CONF", "Hotspot defaults applied");
}
