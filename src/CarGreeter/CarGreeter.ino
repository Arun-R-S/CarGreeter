#include <Arduino.h>
#include <WiFi.h>

#include <esp_err.h>
#include <nvs_flash.h>

#include "src/core/auth_manager.h"
#include "src/core/event_bus.h"
#include "src/core/logger.h"
#include "src/core/network_manager.h"
#include "src/core/system_manager.h"
#include "src/core/time_manager.h"

#include "src/app/config_manager.h"
#include "src/app/jq6500_player.h"
#include "src/app/scheduler.h"
#include "src/app/web_server.h"

namespace {

constexpr const char* kApSsid = "CarGreeter";
// Minimum 8 chars for WPA2. Change this.
constexpr const char* kApPassword = "car12345";

constexpr const char* kDefaultAdminUser = "admin";
constexpr const char* kDefaultAdminPass = "1234";

bool initNvs() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_OK) {
    logInfo("NVS", "Initialized");
    return true;
  }

  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    logWarn("NVS", "Init requires erase");
    const esp_err_t eraseErr = nvs_flash_erase();
    if (eraseErr != ESP_OK) {
      logError("NVS", esp_err_to_name(eraseErr));
      return false;
    }
    err = nvs_flash_init();
    if (err == ESP_OK) {
      logInfo("NVS", "Initialized after erase");
      return true;
    }
  }

  logError("NVS", esp_err_to_name(err));
  return false;
}

bool connectWifiSta(const char* ssid, const char* password) {
  if (ssid == nullptr || password == nullptr || ssid[0] == '\0' || password[0] == '\0') {
    logWarn("WIFI", "Credentials missing; skipping STA");
    return false;
  }
  logInfo("WIFI", "Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 15000u) {
    vTaskDelay(pdMS_TO_TICKS(200));
  }

  if (WiFi.status() == WL_CONNECTED) {
    logInfo("WIFI", "Connected");
    (void)eventBusSend(EVENT_TIME_SYNC_NTP, 0);
    return true;
  }

  logWarn("WIFI", "Not connected (timeout)");
  return false;
}

bool startHotspotAp() {
  configManagerEnsureHotspotCredentials(kApSsid, kApPassword);
  char apSsid[33];
  char apPassword[65];
  configManagerCopyHotspotCredentials(apSsid, sizeof(apSsid), apPassword, sizeof(apPassword));

  logWarn("WIFI", "Starting hotspot (AP) mode");
  WiFi.mode(WIFI_AP);
  const bool ok = WiFi.softAP(apSsid, apPassword);
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
  if (!configManagerHasWifiCredentials()) {
    logWarn("WIFI", "No saved WiFi credentials; starting hotspot");
    return startHotspotAp();
  }

  char ssid[33];
  char password[65];
  configManagerCopyWifiCredentials(ssid, sizeof(ssid), password, sizeof(password));

  if (connectWifiSta(ssid, password)) {
    return true;
  }

  logWarn("WIFI", "STA connect failed; starting hotspot");
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
  (void)initNvs();
  // Default wall clock reference at boot (IST):
  // Jan 01 2026 11:00 AM IST == 2026-01-01 05:30:00Z.
  timeManagerInit(1767245400ULL * 1000ULL);
  timeManagerStartTask();
  configManagerInit();
  configManagerStartTask();

  // Start audio as early as possible so welcome playback is not delayed by WiFi connect/hotspot setup.
  const Jq6500Config jqCfg{
      // IMPORTANT:
      // - Avoid UART1 defaults (GPIO9/10 are SPI flash pins on many ESP32 variants).
      // - Avoid GPIO16/17 on ESP32-WROVER/ESP32-CAM builds that use PSRAM (these pins are commonly wired to PSRAM).
      // - On many ESP32-CAM (AI Thinker style) boards, GPIO4 drives the bright flash LED.
      //   Using it for UART TX will keep the LED on; prefer an SD pin (GPIO13/14) if SD is unused.
      // - TX-only wiring is enough for basic playback control: ESP32 TX -> JQ6500 RX; leave JQ6500 TX unconnected.
      .txPin = 13,
      .rxPin = -1,
      .baudRate = 9600,
      .welcomeTrackIndex = configManagerGetEffectiveTrackIndex(),
      .volume = configManagerGetVolume(),
  };
  jq6500PlayerInit(jqCfg);
  jq6500PlayerStartTask();
  schedulerStartTask();

  systemManagerStartTask();
  networkManagerInit();
  networkManagerStartTask();

  configManagerEnsureAdminCredentials(kDefaultAdminUser, kDefaultAdminPass);
  char adminUser[33];
  char adminPass[65];
  configManagerCopyAdminCredentials(adminUser, sizeof(adminUser), adminPass, sizeof(adminPass));
  authManagerInit(adminUser, adminPass);

  webServerInit();

  const bool networkOk = ensureNetwork();
  if (networkOk) {
    webServerStartTask();
  } else {
    logError("WEB", "Network down; web server not started");
  }

  logInfo("SYS", "System started");
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
