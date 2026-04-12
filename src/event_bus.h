#pragma once

#include <Arduino.h>
#include <stdint.h>

enum EventType : uint8_t {
  EVENT_PLAY = 0,
  EVENT_SET_DELAY = 1,
  EVENT_UPLOAD_DONE = 2,
  EVENT_STOP = 3,
  EVENT_VOLUME_CHANGE = 4,
  EVENT_ERROR = 5,
  EVENT_MAX
};

struct Event {
  EventType type;
  int32_t value;
};

using EventHandler = void (*)(const Event& event, void* context);

void eventBusInit();
void eventBusStartTask(UBaseType_t priority = 2, uint32_t stackWords = 4096);

bool eventBusRegisterHandler(EventType type, EventHandler handler, void* context);
bool eventBusSend(EventType type, int32_t value = 0);

