#include "scheduler.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "config_manager.h"
#include "event_bus.h"
#include "logger.h"

namespace {

TaskHandle_t g_schedulerTask = nullptr;

void schedulerTask(void*) {
  const int32_t delaySeconds = configManagerGetDelaySeconds();
  if (delaySeconds > 0) {
    logInfo("SCHED", "Boot delay started");
    vTaskDelay(pdMS_TO_TICKS(static_cast<uint32_t>(delaySeconds) * 1000u));
  }
  (void)eventBusSend(EVENT_PLAY, 0);
  logInfo("SCHED", "Boot playback event sent");
  g_schedulerTask = nullptr;
  vTaskDelete(nullptr);
}

}

void schedulerInit() {}

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
