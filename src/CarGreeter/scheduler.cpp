#include "scheduler.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "event_bus.h"
#include "logger.h"

namespace {

TaskHandle_t g_schedulerTask = nullptr;
SemaphoreHandle_t g_delayReady = nullptr;
volatile int32_t g_delaySeconds = 5;
volatile bool g_delaySeen = false;

void onSetDelay(const Event& event, void*) {
  int32_t v = event.value;
  if (v < 0) {
    v = 0;
  }
  if (v > 3600) {
    v = 3600;
  }
  g_delaySeconds = v;
  if (!g_delaySeen) {
    g_delaySeen = true;
    if (g_delayReady != nullptr) {
      xSemaphoreGive(g_delayReady);
    }
  }
}

void schedulerTask(void*) {
  if (g_delayReady != nullptr) {
    (void)xSemaphoreTake(g_delayReady, portMAX_DELAY);
  }

  const int32_t delaySeconds = g_delaySeconds;
  if (delaySeconds > 0) {
    char msg[64];
    snprintf(msg, sizeof(msg), "Boot delay running: %ld s", static_cast<long>(delaySeconds));
    logInfo("SCHED", msg);
    vTaskDelay(pdMS_TO_TICKS(static_cast<uint32_t>(delaySeconds) * 1000u));
  } else {
    logInfo("SCHED", "Boot delay skipped (0 s)");
  }
  (void)eventBusSend(EVENT_PLAY, 0);
  logInfo("SCHED", "Boot playback event sent");
  g_schedulerTask = nullptr;
  vTaskDelete(nullptr);
}

}

void schedulerInit() {
  if (g_delayReady == nullptr) {
    g_delayReady = xSemaphoreCreateBinary();
  }
  (void)eventBusRegisterHandler(EVENT_SET_DELAY, onSetDelay, nullptr);
}

void schedulerStartTask(UBaseType_t priority, uint32_t stackWords) {
  if (g_schedulerTask != nullptr) {
    return;
  }
  const BaseType_t ok =
      xTaskCreate(schedulerTask, "scheduler", stackWords, nullptr, priority, &g_schedulerTask);
  if (ok != pdPASS) {
    g_schedulerTask = nullptr;
    logError("SCHED", "Failed to start scheduler task");
  }
}
