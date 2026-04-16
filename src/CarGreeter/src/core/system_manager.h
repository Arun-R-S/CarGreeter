#pragma once

#include <Arduino.h>

void systemManagerInit();
void systemManagerStartTask(UBaseType_t priority = 1, uint32_t stackWords = 4096);

/**
 * Register a task handle for health monitoring in SysInfo.
 */
void systemManagerRegisterTask(TaskHandle_t handle, const char* name);

/**
 * Internal access for Web Server to build the task list.
 */
struct RegisteredTask {
  TaskHandle_t handle;
  char name[16];
};
uint8_t systemManagerGetRegisteredTasks(RegisteredTask* outArray, uint8_t maxCount);
