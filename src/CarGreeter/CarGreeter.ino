#include <Arduino.h>
#include <WiFi.h>

#include "auth_manager.h"
#include "config_manager.h"
#include "event_bus.h"
#include "jq6500_player.h"
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
  Serial.begin(115200);
  Serial.println("Car Greeter Starting...");
  loggerInit();
  loggerStartTask();

  eventBusInit();
  eventBusStartTask();

  schedulerInit();
  configManagerInit();
  authManagerInit("admin", "1234");

  const Jq6500Config jqCfg{
      // ESP32-CAM note: avoid Serial1 defaults (GPIO9/10 are flash pins).
      // TX-only wiring is enough for basic playback control:
      // ESP32 GPIO16 (TX2) -> JQ6500 RX, and keep JQ6500 TX unconnected.
      .txPin = 16,
      .rxPin = -1,
      .baudRate = 9600,
      .welcomeTrackIndex = 1,
      .volume = 20,
  };
  jq6500PlayerInit(jqCfg);
  webServerInit();

  connectWifi();

  jq6500PlayerStartTask();
  webServerStartTask();
  schedulerStartTask();

  logInfo("SYS", "System started");
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
