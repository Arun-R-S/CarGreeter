#include "event_bus.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <string.h>

#include "logger.h"
#include "system_manager.h"

namespace {

constexpr size_t kEventQueueLength = 20;
constexpr size_t kMaxSubscribersPerEvent = 4;

QueueHandle_t g_eventQueue = nullptr;
TaskHandle_t g_dispatchTask = nullptr;

struct Subscriber {
  EventHandler handler;
  void* context;
};

Subscriber g_subscribers[EVENT_MAX][kMaxSubscribersPerEvent] = {};
uint8_t g_subscriberCount[EVENT_MAX] = {};

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

    const uint8_t count = g_subscriberCount[typeIndex];
    if (count == 0) {
      logWarn("EVENT", "No handler registered");
      continue;
    }
    for (uint8_t i = 0; i < count; i++) {
      const Subscriber& s = g_subscribers[typeIndex][i];
      if (s.handler == nullptr) {
        continue;
      }
      s.handler(event, s.context);
    }
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
  } else {
    systemManagerRegisterTask(g_dispatchTask, "event_bus");
  }
}

bool eventBusRegisterHandler(EventType type, EventHandler handler, void* context) {
  const uint8_t typeIndex = static_cast<uint8_t>(type);
  if (typeIndex >= EVENT_MAX) {
    return false;
  }
  if (handler == nullptr) {
    return false;
  }
  uint8_t& count = g_subscriberCount[typeIndex];
  if (count >= kMaxSubscribersPerEvent) {
    logWarn("EVENT", "Subscriber list full");
    return false;
  }
  g_subscribers[typeIndex][count] = Subscriber{handler, context};
  count++;
  return true;
}

bool eventBusSend(EventType type, int32_t value) {
  if (g_eventQueue == nullptr) {
    return false;
  }
  const Event event{type, value, {}, {}};
  const BaseType_t ok = xQueueSend(g_eventQueue, &event, 0);
  if (ok != pdTRUE) {
    logWarn("EVENT", "Queue full; event dropped");
    return false;
  }
  return true;
}

bool eventBusSendText(EventType type, int32_t value, const char* text1, const char* text2) {
  if (g_eventQueue == nullptr) {
    return false;
  }
  Event event{type, value, {}, {}};
  if (text1 != nullptr) {
    snprintf(event.text1, sizeof(event.text1), "%s", text1);
  }
  if (text2 != nullptr) {
    snprintf(event.text2, sizeof(event.text2), "%s", text2);
  }
  const BaseType_t ok = xQueueSend(g_eventQueue, &event, 0);
  if (ok != pdTRUE) {
    logWarn("EVENT", "Queue full; event dropped");
    return false;
  }
  return true;
}
