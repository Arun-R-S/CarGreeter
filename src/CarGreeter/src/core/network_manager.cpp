#include "network_manager.h"

#include <WiFi.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <string.h>

#include "event_bus.h"
#include "logger.h"
#include "system_manager.h"

namespace {

struct ScanResult {
  char ssid[33];
  int16_t rssi;
  uint8_t enc;
};

constexpr size_t kMaxResults = 12;

ScanResult g_results[kMaxResults] = {};
size_t g_count = 0;
volatile bool g_scanRequested = false;
volatile bool g_scanning = false;
uint32_t g_lastScanMs = 0;

SemaphoreHandle_t g_lock = nullptr;
TaskHandle_t g_task = nullptr;

void onScan(const Event&, void*) { g_scanRequested = true; }

void copyResultsLocked(char* out, size_t outSize) {
  if (out == nullptr || outSize == 0) {
    return;
  }
  size_t written = 0;
  out[0] = '\0';

  const auto append = [&](const char* s) {
    if (s == nullptr) {
      return;
    }
    const size_t n = strlen(s);
    if (n >= (outSize - written)) {
      return;
    }
    memcpy(out + written, s, n);
    written += n;
    out[written] = '\0';
  };

  append("[");
  for (size_t i = 0; i < g_count; i++) {
    char item[128];
    const ScanResult& r = g_results[i];
    snprintf(item, sizeof(item), "%s{\"ssid\":\"%s\",\"rssi\":%d,\"enc\":%u}", (i == 0) ? "" : ",",
             r.ssid, static_cast<int>(r.rssi), static_cast<unsigned>(r.enc));
    append(item);
  }
  append("]");
}

void taskFn(void*) {
  for (;;) {
    if (g_scanRequested && !g_scanning) {
      g_scanRequested = false;
      g_scanning = true;
      logDebug("NET", "WiFi scan started");
      (void)WiFi.scanNetworks(true /*async*/, true /*show_hidden*/);
    }

    if (g_scanning) {
      const int n = WiFi.scanComplete();
      if (n >= 0) {
        if (g_lock != nullptr) {
          (void)xSemaphoreTake(g_lock, portMAX_DELAY);
        }

        g_count = 0;
        const int limit = (n < static_cast<int>(kMaxResults)) ? n : static_cast<int>(kMaxResults);
        for (int i = 0; i < limit; i++) {
          ScanResult& r = g_results[g_count];
          snprintf(r.ssid, sizeof(r.ssid), "%s", WiFi.SSID(i).c_str());
          r.rssi = static_cast<int16_t>(WiFi.RSSI(i));
          r.enc = static_cast<uint8_t>(WiFi.encryptionType(i));
          g_count++;
        }
        g_lastScanMs = millis();

        if (g_lock != nullptr) {
          xSemaphoreGive(g_lock);
        }

        WiFi.scanDelete();
        g_scanning = false;
        logDebug("NET", "WiFi scan complete");
      }
    }

    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

}  // namespace

void networkManagerInit() {
  if (g_lock == nullptr) {
    g_lock = xSemaphoreCreateMutex();
  }
  (void)eventBusRegisterHandler(EVENT_WIFI_SCAN, onScan, nullptr);
}

void networkManagerStartTask(UBaseType_t priority, uint32_t stackWords) {
  if (g_task != nullptr) {
    return;
  }
  const BaseType_t ok = xTaskCreate(taskFn, "net", stackWords, nullptr, priority, &g_task);
  if (ok != pdPASS) {
    g_task = nullptr;
    logError("WIFI", "Failed to start network task");
  } else {
    systemManagerRegisterTask(g_task, "network");
  }
}

void networkManagerCopyScanJson(char* out, size_t outSize) {
  if (out == nullptr || outSize == 0) {
    return;
  }

  if (g_lock != nullptr) {
    if (xSemaphoreTake(g_lock, pdMS_TO_TICKS(50)) == pdTRUE) {
      copyResultsLocked(out, outSize);
      xSemaphoreGive(g_lock);
      return;
    }
  }

  // If lock contention, return empty list quickly.
  snprintf(out, outSize, "[]");
}
