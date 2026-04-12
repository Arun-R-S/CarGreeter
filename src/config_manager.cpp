#include "config_manager.h"

#include <SPIFFS.h>
#include <stdlib.h>

#include "event_bus.h"
#include "logger.h"

namespace {

constexpr const char* kConfigPath = "/config.txt";
constexpr int32_t kDefaultDelaySeconds = 5;

volatile int32_t g_delaySeconds = kDefaultDelaySeconds;

void saveDelay(int32_t delaySeconds) {
  File f = SPIFFS.open(kConfigPath, FILE_WRITE);
  if (!f) {
    logWarn("CONF", "Config write failed");
    return;
  }
  f.printf("%ld\n", static_cast<long>(delaySeconds));
  f.close();
}

void loadDelay() {
  if (!SPIFFS.exists(kConfigPath)) {
    g_delaySeconds = kDefaultDelaySeconds;
    return;
  }
  File f = SPIFFS.open(kConfigPath, FILE_READ);
  if (!f) {
    g_delaySeconds = kDefaultDelaySeconds;
    return;
  }
  char buf[24]{};
  const size_t n = f.readBytesUntil('\n', buf, sizeof(buf) - 1);
  buf[n] = '\0';
  f.close();

  const long v = strtol(buf, nullptr, 10);
  if (v < 0) {
    g_delaySeconds = 0;
  } else if (v > 3600) {
    g_delaySeconds = 3600;
  } else {
    g_delaySeconds = static_cast<int32_t>(v);
  }
}

void onSetDelay(const Event& event, void*) {
  int32_t v = event.value;
  if (v < 0) {
    v = 0;
  }
  if (v > 3600) {
    v = 3600;
  }
  g_delaySeconds = v;
  saveDelay(v);
  logInfo("CONF", "Delay updated");
}

}

void configManagerInit() {
  loadDelay();
  (void)eventBusRegisterHandler(EVENT_SET_DELAY, onSetDelay, nullptr);
}

int32_t configManagerGetDelaySeconds() { return g_delaySeconds; }
