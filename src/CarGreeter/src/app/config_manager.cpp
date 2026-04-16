#include "config_manager.h"

#include <Preferences.h>
#include <stddef.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "../core/event_bus.h"
#include "../core/logger.h"
#include "../core/system_manager.h"

namespace {

constexpr char kPrefsNamespace[] = "car_greeter";
constexpr char kKeySlotA[] = "cfgA";
constexpr char kKeySlotB[] = "cfgB";

constexpr uint32_t kMagic = 0x43524754u;  // 'CRGT'
constexpr uint16_t kVersion = 5;

constexpr int32_t kDefaultDelaySeconds = 5;
constexpr int32_t kDefaultVolume = 20;
constexpr uint16_t kDefaultWelcomeTrackIndex = 1;
constexpr uint16_t kDefaultCustomTrackIndex = 0;

constexpr size_t kWifiSsidMaxLen = 32;
constexpr size_t kWifiPasswordMaxLen = 64;

constexpr size_t kApSsidMaxLen = 32;
constexpr size_t kApPasswordMaxLen = 64;
constexpr size_t kAdminUsernameMaxLen = 32;
constexpr size_t kAdminPasswordMaxLen = 64;

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

struct SettingsV4 {
  int32_t delaySeconds;
  int32_t volume;
  uint16_t welcomeTrackIndex;
  uint16_t customTrackIndex;
  char wifiSsid[kWifiSsidMaxLen + 1];
  char wifiPassword[kWifiPasswordMaxLen + 1];
  char hotspotSsid[kApSsidMaxLen + 1];
  char hotspotPassword[kApPasswordMaxLen + 1];
};

struct SettingsRecordV4 {
  uint32_t magic;
  uint16_t version;
  uint16_t size;
  uint32_t seq;
  SettingsV4 settings;
  uint32_t crc32;
};

struct SettingsV5 {
  int32_t delaySeconds;
  int32_t volume;
  uint16_t welcomeTrackIndex;
  uint16_t customTrackIndex;
  char wifiSsid[kWifiSsidMaxLen + 1];
  char wifiPassword[kWifiPasswordMaxLen + 1];
  char hotspotSsid[kApSsidMaxLen + 1];
  char hotspotPassword[kApPasswordMaxLen + 1];
  char adminUsername[kAdminUsernameMaxLen + 1];
  char adminPassword[kAdminPasswordMaxLen + 1];
};

struct SettingsRecordV5 {
  uint32_t magic;
  uint16_t version;
  uint16_t size;
  uint32_t seq;
  SettingsV5 settings;
  uint32_t crc32;
};

TaskHandle_t g_task = nullptr;

// NOTE: Keep initialization compatible with the Arduino ESP32 toolchain (C++11).
SettingsV5 g_settings = {
    kDefaultDelaySeconds,
    kDefaultVolume,
    kDefaultWelcomeTrackIndex,
    kDefaultCustomTrackIndex,
    {0},
    {0},
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

SettingsV5 clampSettings(SettingsV5 s) {
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
  // 0 disables custom override.
  s.wifiSsid[kWifiSsidMaxLen] = '\0';
  s.wifiPassword[kWifiPasswordMaxLen] = '\0';
  s.hotspotSsid[kApSsidMaxLen] = '\0';
  s.hotspotPassword[kApPasswordMaxLen] = '\0';
  s.adminUsername[kAdminUsernameMaxLen] = '\0';
  s.adminPassword[kAdminPasswordMaxLen] = '\0';
  return s;
}

SettingsV4 clampSettingsV4(SettingsV4 s) {
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
  // 0 disables custom override.
  s.wifiSsid[kWifiSsidMaxLen] = '\0';
  s.wifiPassword[kWifiPasswordMaxLen] = '\0';
  s.hotspotSsid[kApSsidMaxLen] = '\0';
  s.hotspotPassword[kApPasswordMaxLen] = '\0';
  return s;
}

bool loadSlotV5(Preferences& prefs, const char* key, SettingsRecordV5* out) {
  if (out == nullptr || key == nullptr) {
    return false;
  }
  const size_t len = prefs.getBytesLength(key);
  if (len != sizeof(SettingsRecordV5)) {
    return false;
  }
  const size_t n = prefs.getBytes(key, out, sizeof(SettingsRecordV5));
  if (n != sizeof(SettingsRecordV5)) {
    return false;
  }
  if (out->magic != kMagic || out->version != 5 || out->size != sizeof(SettingsV5)) {
    return false;
  }
  const uint32_t expected = crc32(reinterpret_cast<const uint8_t*>(out), offsetof(SettingsRecordV5, crc32));
  if (expected != out->crc32) {
    return false;
  }
  out->settings = clampSettings(out->settings);
  return true;
}

SettingsV3 clampSettingsV3(SettingsV3 s) {
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

bool loadSlotV4(Preferences& prefs, const char* key, SettingsRecordV4* out) {
  if (out == nullptr || key == nullptr) {
    return false;
  }
  const size_t len = prefs.getBytesLength(key);
  if (len != sizeof(SettingsRecordV4)) {
    return false;
  }
  const size_t n = prefs.getBytes(key, out, sizeof(SettingsRecordV4));
  if (n != sizeof(SettingsRecordV4)) {
    return false;
  }
  if (out->magic != kMagic || out->version != 4 || out->size != sizeof(SettingsV4)) {
    return false;
  }
  const uint32_t expected = crc32(reinterpret_cast<const uint8_t*>(out), offsetof(SettingsRecordV4, crc32));
  if (expected != out->crc32) {
    return false;
  }
  out->settings = clampSettingsV4(out->settings);
  return true;
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
  out->settings = clampSettingsV3(out->settings);
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

SettingsV5 migrateFromV2(const SettingsV2& v2) {
  SettingsV5 v5{};
  v5.delaySeconds = v2.delaySeconds;
  v5.volume = v2.volume;
  v5.welcomeTrackIndex = v2.welcomeTrackIndex;
  v5.customTrackIndex = kDefaultCustomTrackIndex;
  snprintf(v5.wifiSsid, sizeof(v5.wifiSsid), "%s", v2.wifiSsid);
  snprintf(v5.wifiPassword, sizeof(v5.wifiPassword), "%s", v2.wifiPassword);
  v5.hotspotSsid[0] = '\0';
  v5.hotspotPassword[0] = '\0';
  v5.adminUsername[0] = '\0';
  v5.adminPassword[0] = '\0';
  return clampSettings(v5);
}

SettingsV5 migrateFromV3(const SettingsV3& v3) {
  SettingsV5 v5{};
  v5.delaySeconds = v3.delaySeconds;
  v5.volume = v3.volume;
  v5.welcomeTrackIndex = v3.welcomeTrackIndex;
  v5.customTrackIndex = kDefaultCustomTrackIndex;
  snprintf(v5.wifiSsid, sizeof(v5.wifiSsid), "%s", v3.wifiSsid);
  snprintf(v5.wifiPassword, sizeof(v5.wifiPassword), "%s", v3.wifiPassword);
  snprintf(v5.hotspotSsid, sizeof(v5.hotspotSsid), "%s", v3.hotspotSsid);
  snprintf(v5.hotspotPassword, sizeof(v5.hotspotPassword), "%s", v3.hotspotPassword);
  v5.adminUsername[0] = '\0';
  v5.adminPassword[0] = '\0';
  return clampSettings(v5);
}

SettingsV5 migrateFromV4(const SettingsV4& v4) {
  SettingsV5 v5{};
  v5.delaySeconds = v4.delaySeconds;
  v5.volume = v4.volume;
  v5.welcomeTrackIndex = v4.welcomeTrackIndex;
  v5.customTrackIndex = v4.customTrackIndex;
  snprintf(v5.wifiSsid, sizeof(v5.wifiSsid), "%s", v4.wifiSsid);
  snprintf(v5.wifiPassword, sizeof(v5.wifiPassword), "%s", v4.wifiPassword);
  snprintf(v5.hotspotSsid, sizeof(v5.hotspotSsid), "%s", v4.hotspotSsid);
  snprintf(v5.hotspotPassword, sizeof(v5.hotspotPassword), "%s", v4.hotspotPassword);
  v5.adminUsername[0] = '\0';
  v5.adminPassword[0] = '\0';
  return clampSettings(v5);
}

bool loadBestFromNvs(SettingsV5* outSettings, uint32_t* outSeq, bool* outLastSlotWasA) {
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

  SettingsRecordV5 recA5{};
  SettingsRecordV5 recB5{};
  SettingsRecordV4 recA4{};
  SettingsRecordV4 recB4{};
  SettingsRecordV2 recA2{};
  SettingsRecordV2 recB2{};
  SettingsRecordV3 recA3{};
  SettingsRecordV3 recB3{};

  const bool okA5 = loadSlotV5(prefs, kKeySlotA, &recA5);
  const bool okB5 = loadSlotV5(prefs, kKeySlotB, &recB5);

  const bool okA4 = okA5 ? false : loadSlotV4(prefs, kKeySlotA, &recA4);
  const bool okB4 = okB5 ? false : loadSlotV4(prefs, kKeySlotB, &recB4);

  const bool okA3 = okA4 ? false : loadSlotV3(prefs, kKeySlotA, &recA3);
  const bool okB3 = okB4 ? false : loadSlotV3(prefs, kKeySlotB, &recB3);

  const bool okA = okA5 ? true : (okA4 ? true : (okA3 ? true : loadSlotV2(prefs, kKeySlotA, &recA2)));
  const bool okB = okB5 ? true : (okB4 ? true : (okB3 ? true : loadSlotV2(prefs, kKeySlotB, &recB2)));
  prefs.end();

  if (!okA && !okB) {
    return false;
  }

  const uint32_t seqA = okA5 ? recA5.seq : (okA4 ? recA4.seq : (okA3 ? recA3.seq : recA2.seq));
  const uint32_t seqB = okB5 ? recB5.seq : (okB4 ? recB4.seq : (okB3 ? recB3.seq : recB2.seq));

  if (okA && (!okB || seqA >= seqB)) {
    *outSettings = okA5 ? recA5.settings : (okA4 ? migrateFromV4(recA4.settings) : (okA3 ? migrateFromV3(recA3.settings) : migrateFromV2(recA2.settings)));
    *outSeq = seqA;
    *outLastSlotWasA = true;
    return true;
  }

  *outSettings = okB5 ? recB5.settings : (okB4 ? migrateFromV4(recB4.settings) : (okB3 ? migrateFromV3(recB3.settings) : migrateFromV2(recB2.settings)));
  *outSeq = seqB;
  *outLastSlotWasA = false;
  return true;
}

bool saveToNvsSlot(const SettingsV5& settings, uint32_t seq, bool slotA) {
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, false)) {
    logWarn("CONF", "NVS begin failed");
    return false;
  }

  SettingsRecordV5 rec{};
  rec.magic = kMagic;
  rec.version = kVersion;
  rec.size = sizeof(SettingsV5);
  rec.seq = seq;
  rec.settings = clampSettings(settings);
  rec.crc32 = crc32(reinterpret_cast<const uint8_t*>(&rec), offsetof(SettingsRecordV5, crc32));

  const char* key = slotA ? kKeySlotA : kKeySlotB;
  const size_t written = prefs.putBytes(key, &rec, sizeof(SettingsRecordV5));
  prefs.end();
  return written == sizeof(SettingsRecordV5);
}

void scheduleSave() {
  g_dirty = true;
  g_lastChangeMs = millis();
  if (g_task != nullptr) {
    xTaskNotifyGive(g_task);
  }
}

void scheduleSaveImmediate() {
  g_dirty = true;
  g_lastChangeMs = millis() - kDebounceMs;
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

void onWifiSet(const Event& event, void*) {
  if (event.text1[0] == '\0' || event.text2[0] == '\0') {
    logWarn("CONF", "WiFi credentials empty; ignoring");
    return;
  }
  if (strncmp(g_settings.wifiSsid, event.text1, sizeof(g_settings.wifiSsid)) == 0 &&
      strncmp(g_settings.wifiPassword, event.text2, sizeof(g_settings.wifiPassword)) == 0) {
    return;
  }
  snprintf(g_settings.wifiSsid, sizeof(g_settings.wifiSsid), "%s", event.text1);
  snprintf(g_settings.wifiPassword, sizeof(g_settings.wifiPassword), "%s", event.text2);
  scheduleSaveImmediate();
  logInfo("CONF", "WiFi credentials updated");
}

void onWifiForget(const Event&, void*) {
  if (g_settings.wifiSsid[0] == '\0' && g_settings.wifiPassword[0] == '\0') {
    return;
  }
  g_settings.wifiSsid[0] = '\0';
  g_settings.wifiPassword[0] = '\0';
  scheduleSaveImmediate();
  logInfo("CONF", "WiFi credentials cleared");
}

void onHotspotSet(const Event& event, void*) {
  if (event.text1[0] == '\0' || event.text2[0] == '\0') {
    logWarn("CONF", "Hotspot credentials empty; ignoring");
    return;
  }
  if (strncmp(g_settings.hotspotSsid, event.text1, sizeof(g_settings.hotspotSsid)) == 0 &&
      strncmp(g_settings.hotspotPassword, event.text2, sizeof(g_settings.hotspotPassword)) == 0) {
    return;
  }
  snprintf(g_settings.hotspotSsid, sizeof(g_settings.hotspotSsid), "%s", event.text1);
  snprintf(g_settings.hotspotPassword, sizeof(g_settings.hotspotPassword), "%s", event.text2);
  scheduleSaveImmediate();
  logInfo("CONF", "Hotspot credentials updated");
}

void onAudioSetPreloaded(const Event& event, void*) {
  int32_t v = event.value;
  if (v < 1) {
    v = 1;
  }
  if (v > 9999) {
    v = 9999;
  }
  const uint16_t idx = static_cast<uint16_t>(v);
  if (idx == g_settings.welcomeTrackIndex) {
    return;
  }
  g_settings.welcomeTrackIndex = idx;
  scheduleSave();
  logInfo("CONF", "Preloaded audio index updated");
}

void onAudioSetCustom(const Event& event, void*) {
  int32_t v = event.value;
  if (v < 0) {
    v = 0;
  }
  if (v > 9999) {
    v = 9999;
  }
  const uint16_t idx = static_cast<uint16_t>(v);
  if (idx == g_settings.customTrackIndex) {
    return;
  }
  g_settings.customTrackIndex = idx;
  scheduleSave();
  logInfo("CONF", "Custom audio index updated");
}

void onVolumeChange(const Event& event, void*) {
  int32_t v = event.value;
  if (v < 0) {
    v = 0;
  }
  if (v > 30) {
    v = 30;
  }
  if (v == g_settings.volume) {
    return;
  }
  g_settings.volume = v;
  scheduleSave();
  logInfo("CONF", "Volume updated");
}

void onAuthSet(const Event& event, void*) {
  if (event.text1[0] == '\0' || event.text2[0] == '\0') {
    logWarn("CONF", "Auth credentials empty; ignoring");
    return;
  }
  if (strncmp(g_settings.adminUsername, event.text1, sizeof(g_settings.adminUsername)) == 0 &&
      strncmp(g_settings.adminPassword, event.text2, sizeof(g_settings.adminPassword)) == 0) {
    return;
  }
  snprintf(g_settings.adminUsername, sizeof(g_settings.adminUsername), "%s", event.text1);
  snprintf(g_settings.adminPassword, sizeof(g_settings.adminPassword), "%s", event.text2);
  scheduleSaveImmediate();
  logInfo("CONF", "Admin credentials updated");
}

void onFactoryResetEvent(const Event&, void*) {
  configManagerFactoryReset();
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
  (void)eventBusRegisterHandler(EVENT_WIFI_SET, onWifiSet, nullptr);
  (void)eventBusRegisterHandler(EVENT_WIFI_FORGET, onWifiForget, nullptr);
  (void)eventBusRegisterHandler(EVENT_HOTSPOT_SET, onHotspotSet, nullptr);
  (void)eventBusRegisterHandler(EVENT_AUDIO_SET_PRELOADED, onAudioSetPreloaded, nullptr);
  (void)eventBusRegisterHandler(EVENT_AUDIO_SET_CUSTOM, onAudioSetCustom, nullptr);
  (void)eventBusRegisterHandler(EVENT_VOLUME_CHANGE, onVolumeChange, nullptr);
  (void)eventBusRegisterHandler(EVENT_AUTH_SET, onAuthSet, nullptr);
  (void)eventBusRegisterHandler(EVENT_SYSTEM_FACTORY_RESET, onFactoryResetEvent, nullptr);

  SettingsV5 loaded{};
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
  (void)eventBusSend(EVENT_VOLUME_CHANGE, g_settings.volume);
  (void)eventBusSend(EVENT_AUDIO_SET_PRELOADED, g_settings.welcomeTrackIndex);
  (void)eventBusSend(EVENT_AUDIO_SET_CUSTOM, g_settings.customTrackIndex);
}

void configManagerStartTask(UBaseType_t priority, uint32_t stackWords) {
  if (g_task != nullptr) {
    return;
  }
  const BaseType_t ok = xTaskCreate(configTask, "settings", stackWords, nullptr, priority, &g_task);
  if (ok != pdPASS) {
    g_task = nullptr;
    logError("CONF", "Failed to start settings task");
  } else {
    systemManagerRegisterTask(g_task, "config_mgr");
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
  scheduleSaveImmediate();
  logInfo("CONF", "Hotspot defaults applied");
}

int32_t configManagerGetDelaySeconds() { return g_settings.delaySeconds; }
int32_t configManagerGetVolume() { return g_settings.volume; }
uint16_t configManagerGetPreloadedTrackIndex() { return g_settings.welcomeTrackIndex; }
uint16_t configManagerGetCustomTrackIndex() { return g_settings.customTrackIndex; }
uint16_t configManagerGetEffectiveTrackIndex() {
  return (g_settings.customTrackIndex != 0) ? g_settings.customTrackIndex : g_settings.welcomeTrackIndex;
}

void configManagerCopyWifiSsid(char* out, size_t outSize) {
  if (out == nullptr || outSize == 0) {
    return;
  }
  snprintf(out, outSize, "%s", g_settings.wifiSsid);
}

void configManagerCopyHotspotSsid(char* out, size_t outSize) {
  if (out == nullptr || outSize == 0) {
    return;
  }
  snprintf(out, outSize, "%s", g_settings.hotspotSsid);
}

bool configManagerHasAdminCredentials() {
  return g_settings.adminUsername[0] != '\0' && g_settings.adminPassword[0] != '\0';
}

void configManagerCopyAdminCredentials(char* usernameOut,
                                       size_t usernameOutSize,
                                       char* passwordOut,
                                       size_t passwordOutSize) {
  if (usernameOut != nullptr && usernameOutSize > 0) {
    snprintf(usernameOut, usernameOutSize, "%s", g_settings.adminUsername);
  }
  if (passwordOut != nullptr && passwordOutSize > 0) {
    snprintf(passwordOut, passwordOutSize, "%s", g_settings.adminPassword);
  }
}

void configManagerEnsureAdminCredentials(const char* defaultUsername, const char* defaultPassword) {
  if (configManagerHasAdminCredentials()) {
    return;
  }
  if (defaultUsername == nullptr || defaultPassword == nullptr || defaultUsername[0] == '\0' || defaultPassword[0] == '\0') {
    logWarn("CONF", "Default admin credentials missing");
    return;
  }
  snprintf(g_settings.adminUsername, sizeof(g_settings.adminUsername), "%s", defaultUsername);
  snprintf(g_settings.adminPassword, sizeof(g_settings.adminPassword), "%s", defaultPassword);
  scheduleSaveImmediate();
  logInfo("CONF", "Admin defaults applied");
}

void configManagerFactoryReset() {
  Preferences prefs;
  if (prefs.begin(kPrefsNamespace, false)) {
    (void)prefs.clear();
    prefs.end();
  }

  g_settings = SettingsV5{
      kDefaultDelaySeconds,
      kDefaultVolume,
      kDefaultWelcomeTrackIndex,
      kDefaultCustomTrackIndex,
      {0},
      {0},
      {0},
      {0},
      {0},
      {0},
  };
  g_seq = 0;
  g_lastSlotWasA = true;
  g_dirty = false;
  g_lastChangeMs = 0;
  g_lastSaveMs = 0;
  logWarn("CONF", "Factory reset done");
}
