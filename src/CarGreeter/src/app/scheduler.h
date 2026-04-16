#pragma once

#include <Arduino.h>

void schedulerInit();
void schedulerStartTask(UBaseType_t priority = 2, uint32_t stackWords = 4096);

