#include "time_manager.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_sntp.h>
#include <stdlib.h>
#include <time.h>

#include "event_bus.h"
#include "logger.h"

namespace {

constexpr int32_t kIstOffsetSeconds = 5 * 3600 + 30 * 60;
constexpr uint32_t kSyncMinIntervalMs = 180000u;  // 3 minutes

TaskHandle_t g_task = nullptr;
volatile bool g_ntpRequested = false;
uint32_t g_lastClientSyncAtMs = 0;
uint32_t g_lastNtpSyncAtMs = 0;

portMUX_TYPE g_mux = portMUX_INITIALIZER_UNLOCKED;
uint64_t g_baseEpochMsUtc = 0;  // epoch at baseMillis
uint32_t g_baseMillis = 0;

void setBase(uint64_t nowEpochMsUtc) {
  const uint32_t nowMs = millis();
  portENTER_CRITICAL(&g_mux);
  g_baseEpochMsUtc = nowEpochMsUtc;
  g_baseMillis = nowMs;
  portEXIT_CRITICAL(&g_mux);
}

void onTimeSync(const Event& event, void*) {
  // Expect text1 = epoch ms (UTC) as decimal string.
  if (event.text1[0] == '\0') {
    logWarn("TIME", "Missing epochMs");
    return;
  }
  const uint64_t epochMs = strtoull(event.text1, nullptr, 10);
  if (epochMs < 1000u) {
    logWarn("TIME", "Invalid epochMs");
    return;
  }

  const uint32_t nowMs = millis();
  const uint64_t current = timeManagerNowEpochMsUtc();
  const int64_t diff = static_cast<int64_t>(epochMs) - static_cast<int64_t>(current);
  const uint64_t absDiff = (diff < 0) ? static_cast<uint64_t>(-diff) : static_cast<uint64_t>(diff);

  const bool tooSoon = (g_lastClientSyncAtMs != 0) && ((nowMs - g_lastClientSyncAtMs) < kSyncMinIntervalMs);
  // Always accept if drift is large (manual clock change), otherwise rate-limit.
  if (tooSoon && absDiff < 60000u) {
    logDebug("TIME", "Client sync ignored (rate limited)");
    return;
  }

  setBase(epochMs);
  g_lastClientSyncAtMs = nowMs;
  logInfo("TIME", "Time synced from client");
}

void onNtpRequest(const Event&, void*) {
  const uint32_t nowMs = millis();
  if (g_lastNtpSyncAtMs != 0 && (nowMs - g_lastNtpSyncAtMs) < kSyncMinIntervalMs) {
    return;
  }
  g_ntpRequested = true;
  if (g_task != nullptr) {
    xTaskNotifyGive(g_task);
  }
}

void taskFn(void*) {
  for (;;) {
    (void)ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000));
    if (!g_ntpRequested) {
      continue;
    }
    g_ntpRequested = false;

    // Start SNTP; do not block long.
    configTime(0, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");

    bool ok = false;
    for (int i = 0; i < 20; i++) {
      const time_t t = time(nullptr);
      if (t > 1700000000) {  // sanity check (>= 2023)
        setBase(static_cast<uint64_t>(t) * 1000u);
        ok = true;
        break;
      }
      vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (ok) {
      g_lastNtpSyncAtMs = millis();
      logInfo("TIME", "Time synced from NTP");
    } else {
      logWarn("TIME", "NTP sync failed");
    }
  }
}

}  // namespace

void timeManagerInit(uint64_t bootEpochMsUtc) {
  setBase(bootEpochMsUtc);
  (void)eventBusRegisterHandler(EVENT_TIME_SYNC, onTimeSync, nullptr);
  (void)eventBusRegisterHandler(EVENT_TIME_SYNC_NTP, onNtpRequest, nullptr);
}

void timeManagerStartTask(UBaseType_t priority, uint32_t stackWords) {
  if (g_task != nullptr) {
    return;
  }
  const BaseType_t ok = xTaskCreate(taskFn, "time", stackWords, nullptr, priority, &g_task);
  if (ok != pdPASS) {
    g_task = nullptr;
    logError("TIME", "Failed to start time task");
  }
}

uint64_t timeManagerNowEpochMsUtc() {
  uint64_t baseEpochMsUtc = 0;
  uint32_t baseMillis = 0;
  portENTER_CRITICAL(&g_mux);
  baseEpochMsUtc = g_baseEpochMsUtc;
  baseMillis = g_baseMillis;
  portEXIT_CRITICAL(&g_mux);

  const uint32_t nowMs = millis();
  const uint32_t delta = nowMs - baseMillis;
  return baseEpochMsUtc + static_cast<uint64_t>(delta);
}

void timeManagerFormatIst(uint64_t epochMsUtc, char* out, size_t outSize) {
  if (out == nullptr || outSize == 0) {
    return;
  }

  const uint64_t secUtc = epochMsUtc / 1000u;
  const uint32_t ms = static_cast<uint32_t>(epochMsUtc % 1000u);
  const time_t secIst = static_cast<time_t>(secUtc + static_cast<uint64_t>(kIstOffsetSeconds));

  struct tm tmIst {};
  gmtime_r(&secIst, &tmIst);

  static const char* kMon[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  const int monIdx = (tmIst.tm_mon >= 0 && tmIst.tm_mon < 12) ? tmIst.tm_mon : 0;

  int hour12 = tmIst.tm_hour % 12;
  if (hour12 == 0) {
    hour12 = 12;
  }
  const char* ampm = (tmIst.tm_hour >= 12) ? "PM" : "AM";

  snprintf(out, outSize, "%s %02d %04d %02d:%02d:%02d.%03lu %s IST", kMon[monIdx], tmIst.tm_mday,
           tmIst.tm_year + 1900, hour12, tmIst.tm_min, tmIst.tm_sec, static_cast<unsigned long>(ms), ampm);
}
