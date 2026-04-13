#pragma once

#include <Arduino.h>
#include <stdint.h>

struct Jq6500Config {
  int txPin;
  int rxPin;  // optional; set to -1 if not used
  uint32_t baudRate;
  uint16_t welcomeTrackIndex;
  uint8_t volume;  // 0-30
};

void jq6500PlayerInit(const Jq6500Config& config);
void jq6500PlayerStartTask(UBaseType_t priority = 3, uint32_t stackWords = 4096);
