#pragma once

#include <Arduino.h>
#include <stdint.h>

enum LogLevel : uint8_t { LOG_INFO = 0, LOG_WARN = 1, LOG_ERROR = 2 };

void loggerInit();
void loggerStartTask(UBaseType_t priority = 1, uint32_t stackWords = 4096);

void logInfo(const char* tag, const char* message);
void logWarn(const char* tag, const char* message);
void logError(const char* tag, const char* message);

void loggerCopyRecent(char* out, size_t outSize);

