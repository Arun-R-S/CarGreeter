#pragma once

#include <Arduino.h>
#include <stdint.h>

enum EventType : uint8_t {
  EVENT_PLAY = 0,
  EVENT_SET_DELAY = 1,
  EVENT_STOP = 2,
  EVENT_VOLUME_CHANGE = 3,
  EVENT_ERROR = 4,
  EVENT_WIFI_SET = 5,
  EVENT_WIFI_FORGET = 6,
  EVENT_HOTSPOT_SET = 7,
  EVENT_AUDIO_SET_PRELOADED = 8,
  EVENT_AUDIO_SET_CUSTOM = 9,
  EVENT_SYSTEM_RESTART = 10,
  EVENT_SYSTEM_FACTORY_RESET = 11,
  EVENT_WIFI_SCAN = 12,
  EVENT_TIME_SYNC = 13,
  EVENT_TIME_SYNC_NTP = 14,
  EVENT_MAX
};

constexpr size_t kEventText1Max = 32;
constexpr size_t kEventText2Max = 64;

struct Event {
  EventType type;
  int32_t value;
  char text1[kEventText1Max + 1];
  char text2[kEventText2Max + 1];
};

using EventHandler = void (*)(const Event& event, void* context);

void eventBusInit();
void eventBusStartTask(UBaseType_t priority = 2, uint32_t stackWords = 4096);

bool eventBusRegisterHandler(EventType type, EventHandler handler, void* context);
bool eventBusSend(EventType type, int32_t value = 0);
bool eventBusSendText(EventType type, int32_t value, const char* text1, const char* text2);
