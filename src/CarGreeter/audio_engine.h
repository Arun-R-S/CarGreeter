#pragma once

#include <Arduino.h>

void audioEngineInit();
void audioEngineStartTask(UBaseType_t priority = 3, uint32_t stackWords = 6144);

