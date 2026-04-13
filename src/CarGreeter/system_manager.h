#pragma once

#include <Arduino.h>

void systemManagerInit();
void systemManagerStartTask(UBaseType_t priority = 1, uint32_t stackWords = 4096);

