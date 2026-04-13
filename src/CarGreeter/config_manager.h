#pragma once

#include <Arduino.h>

void configManagerInit();
void configManagerStartTask(UBaseType_t priority = 1, uint32_t stackWords = 4096);
