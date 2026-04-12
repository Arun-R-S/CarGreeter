#pragma once

#include <Arduino.h>

void webServerInit();
void webServerStartTask(UBaseType_t priority = 1, uint32_t stackWords = 6144);

