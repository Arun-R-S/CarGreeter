#pragma once

#include <Arduino.h>
#include <stdint.h>

// Provides a monotonic "wall clock" based on a boot epoch and elapsed millis().
// Displays time as IST (UTC+05:30) regardless of network mode.

void timeManagerInit(uint64_t bootEpochMsUtc);
void timeManagerStartTask(UBaseType_t priority = 1, uint32_t stackWords = 4096);

uint64_t timeManagerNowEpochMsUtc();
void timeManagerFormatIst(uint64_t epochMsUtc, char* out, size_t outSize);

