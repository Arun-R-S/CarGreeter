#include "system_manager.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <esp_system.h>

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

constexpr uint8_t kMaxRegisteredTasks = 16;
RegisteredTask g_registeredTasks[kMaxRegisteredTasks] = {};
uint8_t g_registeredCount = 0;

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
      logWarn("SYS", "Factory reset in progress (settings module will clear data)");
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

void systemManagerRegisterTask(TaskHandle_t handle, const char* name) {
  if (handle == nullptr || g_registeredCount >= kMaxRegisteredTasks) {
    return;
  }
  g_registeredTasks[g_registeredCount].handle = handle;
  snprintf(g_registeredTasks[g_registeredCount].name, sizeof(g_registeredTasks[g_registeredCount].name), "%s", name);
  g_registeredCount++;
}

uint8_t systemManagerGetRegisteredTasks(RegisteredTask* outArray, uint8_t maxCount) {
  if (outArray == nullptr || maxCount == 0) return 0;
  uint8_t count = (g_registeredCount < maxCount) ? g_registeredCount : maxCount;
  for (uint8_t i = 0; i < count; i++) {
    outArray[i] = g_registeredTasks[i];
  }
  return count;
}

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
  } else {
    systemManagerRegisterTask(g_task, "system");
  }
}

