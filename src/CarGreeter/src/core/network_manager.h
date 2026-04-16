#pragma once

#include <Arduino.h>

void networkManagerInit();
void networkManagerStartTask(UBaseType_t priority = 1, uint32_t stackWords = 4096);

// Writes JSON array into out. Always null-terminated when outSize > 0.
// Format: [{"ssid":"...","rssi":-55,"enc":1}, ...]
void networkManagerCopyScanJson(char* out, size_t outSize);

