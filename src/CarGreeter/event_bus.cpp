#include "event_bus.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "logger.h"

namespace {

constexpr size_t kEventQueueLength = 20;

QueueHandle_t g_eventQueue = nullptr;
TaskHandle_t g_dispatchTask = nullptr;
EventHandler g_handlers[EVENT_MAX] = {};
void* g_handlerContexts[EVENT_MAX] = {};

void dispatchTask(void*) {
  Event event{};
  for (;;) {
    if (xQueueReceive(g_eventQueue, &event, portMAX_DELAY) != pdTRUE) {
      continue;
    }

    const uint8_t typeIndex = static_cast<uint8_t>(event.type);
    if (typeIndex >= EVENT_MAX) {
      logWarn("EVENT", "Invalid event type");
      continue;
    }

    EventHandler handler = g_handlers[typeIndex];
    if (handler == nullptr) {
      logWarn("EVENT", "No handler registered");
      continue;
    }
    handler(event, g_handlerContexts[typeIndex]);
  }
}

}

void eventBusInit() {
  if (g_eventQueue != nullptr) {
    return;
  }
  g_eventQueue = xQueueCreate(kEventQueueLength, sizeof(Event));
  if (g_eventQueue == nullptr) {
    logError("EVENT", "Failed to create queue");
  }
}

void eventBusStartTask(UBaseType_t priority, uint32_t stackWords) {
  if (g_eventQueue == nullptr || g_dispatchTask != nullptr) {
    return;
  }
  const BaseType_t ok =
      xTaskCreate(dispatchTask, "event_dispatch", stackWords, nullptr, priority, &g_dispatchTask);
  if (ok != pdPASS) {
    g_dispatchTask = nullptr;
    logError("EVENT", "Failed to start dispatch task");
  }
}

bool eventBusRegisterHandler(EventType type, EventHandler handler, void* context) {
  const uint8_t typeIndex = static_cast<uint8_t>(type);
  if (typeIndex >= EVENT_MAX) {
    return false;
  }
  g_handlers[typeIndex] = handler;
  g_handlerContexts[typeIndex] = context;
  return true;
}

bool eventBusSend(EventType type, int32_t value) {
  if (g_eventQueue == nullptr) {
    return false;
  }
  const Event event{type, value};
  const BaseType_t ok = xQueueSend(g_eventQueue, &event, 0);
  if (ok != pdTRUE) {
    logWarn("EVENT", "Queue full; event dropped");
    return false;
  }
  return true;
}
