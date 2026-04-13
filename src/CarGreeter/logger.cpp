#include "logger.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace {

struct LogEntry {
  LogLevel level;
  char tag[12];
  char message[80];
};

constexpr size_t kLogQueueLength = 40;
constexpr size_t kLogBufferEntries = 25;

QueueHandle_t g_logQueue = nullptr;
TaskHandle_t g_loggerTask = nullptr;
LogEntry g_recent[kLogBufferEntries] = {};
size_t g_recentIndex = 0;
bool g_recentWrapped = false;

const char* levelToString(LogLevel level) {
  switch (level) {
    case LOG_DEBUG:
      return "DEBUG";
    case LOG_INFO:
      return "INFO";
    case LOG_WARN:
      return "WARN";
    case LOG_ERROR:
      return "ERROR";
    default:
      return "UNK";
  }
}

void storeRecent(const LogEntry& entry) {
  g_recent[g_recentIndex] = entry;
  g_recentIndex = (g_recentIndex + 1) % kLogBufferEntries;
  if (g_recentIndex == 0) {
    g_recentWrapped = true;
  }
}

void loggerTask(void*) {
  LogEntry entry{};
  for (;;) {
    if (xQueueReceive(g_logQueue, &entry, portMAX_DELAY) != pdTRUE) {
      continue;
    }
    storeRecent(entry);
    Serial.printf("[%s] %s: %s\r\n", levelToString(entry.level), entry.tag, entry.message);
  }
}

void enqueue(LogLevel level, const char* tag, const char* message) {
  if (g_logQueue == nullptr) {
    return;
  }

  LogEntry entry{};
  entry.level = level;
  snprintf(entry.tag, sizeof(entry.tag), "%s", (tag != nullptr) ? tag : "");
  snprintf(entry.message, sizeof(entry.message), "%s", (message != nullptr) ? message : "");

  (void)xQueueSend(g_logQueue, &entry, 0);
}

}

void loggerInit() {
  if (g_logQueue != nullptr) {
    return;
  }
  Serial.begin(115200);
  g_logQueue = xQueueCreate(kLogQueueLength, sizeof(LogEntry));
}

void loggerStartTask(UBaseType_t priority, uint32_t stackWords) {
  if (g_logQueue == nullptr || g_loggerTask != nullptr) {
    return;
  }

  const BaseType_t ok = xTaskCreate(loggerTask, "logger", stackWords, nullptr, priority, &g_loggerTask);
  if (ok != pdPASS) {
    g_loggerTask = nullptr;
  }
}

void logInfo(const char* tag, const char* message) { enqueue(LOG_INFO, tag, message); }
void logWarn(const char* tag, const char* message) { enqueue(LOG_WARN, tag, message); }
void logError(const char* tag, const char* message) { enqueue(LOG_ERROR, tag, message); }
void logDebug(const char* tag, const char* message) { enqueue(LOG_DEBUG, tag, message); }

void loggerCopyRecent(char* out, size_t outSize) {
  if (out == nullptr || outSize == 0) {
    return;
  }
  out[0] = '\0';

  const size_t count = g_recentWrapped ? kLogBufferEntries : g_recentIndex;
  const size_t start = g_recentWrapped ? g_recentIndex : 0;

  size_t written = 0;
  for (size_t i = 0; i < count; i++) {
    const size_t idx = (start + i) % kLogBufferEntries;
    const LogEntry& entry = g_recent[idx];
    const int n = snprintf(out + written, outSize - written, "[%s] %s: %s\r\n",
                           levelToString(entry.level), entry.tag, entry.message);
    if (n < 0) {
      break;
    }
    const size_t nu = static_cast<size_t>(n);
    if (nu >= (outSize - written)) {
      out[outSize - 1] = '\0';
      break;
    }
    written += nu;
  }
}
