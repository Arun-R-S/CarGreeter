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

constexpr const char* kApSsid = "CarGreeter";
// Minimum 8 chars for WPA2. Change this.
constexpr const char* kApPassword = "car12345";

bool connectWifiSta() {
  logInfo("WIFI", "Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(kWifiSsid, kWifiPassword);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 15000u) {
    vTaskDelay(pdMS_TO_TICKS(200));
  }

  if (WiFi.status() == WL_CONNECTED) {
    logInfo("WIFI", "Connected");
    return true;
  }

  logWarn("WIFI", "Not connected (timeout)");
  return false;
}

bool startHotspotAp() {
  logWarn("WIFI", "Starting hotspot (AP) mode");
  WiFi.mode(WIFI_AP);
  const bool ok = WiFi.softAP(kApSsid, kApPassword);
  if (!ok) {
    logError("WIFI", "Failed to start hotspot");
    return false;
  }
  const IPAddress ip = WiFi.softAPIP();
  char ipBuf[32];
  snprintf(ipBuf, sizeof(ipBuf), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  logInfo("WIFI", "Hotspot started");
  logInfo("WIFI", ipBuf);
  return true;
}

bool ensureNetwork() {
  if (connectWifiSta()) {
    return true;
  }
  return startHotspotAp();
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
  configManagerStartTask();
  authManagerInit("admin", "1234");

  const bool networkOk = ensureNetwork();

  const Jq6500Config jqCfg{
      // IMPORTANT:
      // - Avoid UART1 defaults (GPIO9/10 are SPI flash pins on many ESP32 variants).
      // - Avoid GPIO16/17 on ESP32-WROVER/ESP32-CAM builds that use PSRAM (these pins are commonly wired to PSRAM).
      // - TX-only wiring is enough for basic playback control: ESP32 TX -> JQ6500 RX; leave JQ6500 TX unconnected.
      .txPin = 4,
      .rxPin = -1,
      .baudRate = 9600,
      .welcomeTrackIndex = 1,
      .volume = 20,
  };
  jq6500PlayerInit(jqCfg);
  webServerInit();

  jq6500PlayerStartTask();
  if (networkOk) {
    webServerStartTask();
  } else {
    logError("WEB", "Network down; web server not started");
  }
  schedulerStartTask();

  logInfo("SYS", "System started");
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
