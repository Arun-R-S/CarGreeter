#include <Arduino.h>
#include <WiFi.h>

#include "audio_engine.h"
#include "auth_manager.h"
#include "config_manager.h"
#include "event_bus.h"
#include "file_manager.h"
#include "logger.h"
#include "scheduler.h"
#include "web_server.h"

namespace {

constexpr const char* kWifiSsid = "YOUR_WIFI_SSID";
constexpr const char* kWifiPassword = "YOUR_WIFI_PASSWORD";

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(kWifiSsid, kWifiPassword);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 15000u) {
    vTaskDelay(pdMS_TO_TICKS(200));
  }

  if (WiFi.status() == WL_CONNECTED) {
    logInfo("WIFI", "Connected");
  } else {
    logWarn("WIFI", "Not connected");
  }
}

}

void setup() {
  loggerInit();
  loggerStartTask();

  eventBusInit();

  (void)fileManagerInit();
  configManagerInit();
  authManagerInit("admin", "1234");

  audioEngineInit();
  schedulerInit();
  webServerInit();

  connectWifi();

  audioEngineStartTask();
  eventBusStartTask();
  webServerStartTask();
  schedulerStartTask();

  logInfo("SYS", "System started");
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
