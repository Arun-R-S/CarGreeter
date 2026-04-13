#include "system_manager.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <esp_system.h>

#include "config_manager.h"
#include "event_bus.h"
#include "logger.h"

namespace {

struct SystemCommand {
  EventType type;
  int32_t value;
};

constexpr size_t kQueueLen = 8;

QueueHandle_t g_queue = nullptr;
TaskHandle_t g_task = nullptr;

void onRestart(const Event& event, void*) {
  if (g_queue == nullptr) {
    return;
  }
  const SystemCommand cmd{EVENT_SYSTEM_RESTART, event.value};
  (void)xQueueSend(g_queue, &cmd, 0);
}

void onFactoryReset(const Event& event, void*) {
  if (g_queue == nullptr) {
    return;
  }
  const SystemCommand cmd{EVENT_SYSTEM_FACTORY_RESET, event.value};
  (void)xQueueSend(g_queue, &cmd, 0);
}

void taskFn(void*) {
  SystemCommand cmd{};
  for (;;) {
    if (xQueueReceive(g_queue, &cmd, portMAX_DELAY) != pdTRUE) {
      continue;
    }

    if (cmd.type == EVENT_SYSTEM_FACTORY_RESET) {
      logWarn("SYS", "Factory reset requested");
      configManagerFactoryReset();
    } else if (cmd.type == EVENT_SYSTEM_RESTART) {
      logWarn("SYS", "Restart requested");
    } else {
      continue;
    }

    int32_t delayMs = cmd.value;
    if (delayMs < 0) {
      delayMs = 0;
    }
    if (delayMs > 10000) {
      delayMs = 10000;
    }
    if (delayMs > 0) {
      vTaskDelay(pdMS_TO_TICKS(static_cast<uint32_t>(delayMs)));
    } else {
      vTaskDelay(pdMS_TO_TICKS(250));
    }

    logWarn("SYS", "Rebooting...");
    esp_restart();
  }
}

}  // namespace

void systemManagerInit() {
  if (g_queue == nullptr) {
    g_queue = xQueueCreate(kQueueLen, sizeof(SystemCommand));
  }
  (void)eventBusRegisterHandler(EVENT_SYSTEM_RESTART, onRestart, nullptr);
  (void)eventBusRegisterHandler(EVENT_SYSTEM_FACTORY_RESET, onFactoryReset, nullptr);
}

void systemManagerStartTask(UBaseType_t priority, uint32_t stackWords) {
  if (g_task != nullptr || g_queue == nullptr) {
    return;
  }
  const BaseType_t ok = xTaskCreate(taskFn, "system", stackWords, nullptr, priority, &g_task);
  if (ok != pdPASS) {
    g_task = nullptr;
    logError("SYS", "Failed to start system task");
  }
}

