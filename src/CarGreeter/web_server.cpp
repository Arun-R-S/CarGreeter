#include "web_server.h"

#include <WebServer.h>
#include <WiFi.h>
#include <esp_system.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_mac.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "auth_manager.h"
#include "config_manager.h"
#include "event_bus.h"
#include "logger.h"
#include "network_manager.h"
#include "web_pages.h"

namespace {

WebServer g_server(80);
TaskHandle_t g_webTask = nullptr;
bool g_serverStarted = false;

void handleRoot() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  g_server.send_P(200, "text/html", kIndexHtml);
}

void handleLogView() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  g_server.send_P(200, "text/html", kLogViewHtml);
}

void handlePlay() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  (void)eventBusSend(EVENT_PLAY, 0);
  g_server.send(200, "text/plain", "Playback triggered");
  logInfo("WEB", "EVENT_PLAY sent");
}

void handleSetDelay() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  if (!g_server.hasArg("value")) {
    g_server.send(400, "text/plain", "Missing value");
    return;
  }
  const long v = g_server.arg("value").toInt();
  (void)eventBusSend(EVENT_SET_DELAY, static_cast<int32_t>(v));
  g_server.send(200, "text/plain", "Delay updated");
  logInfo("WEB", "EVENT_SET_DELAY sent");
}

void handleSettings() {
  if (!authManagerEnsure(g_server)) {
    return;
  }

  const bool connected = (WiFi.status() == WL_CONNECTED);
  const String connectedSsid = connected ? WiFi.SSID() : String("");
  const int32_t rssi = connected ? static_cast<int32_t>(WiFi.RSSI()) : 0;

  const IPAddress ip = (WiFi.getMode() == WIFI_AP) ? WiFi.softAPIP() : WiFi.localIP();
  char ipBuf[32];
  snprintf(ipBuf, sizeof(ipBuf), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

  char wifiSsid[33];
  char apSsid[33];
  configManagerCopyWifiSsid(wifiSsid, sizeof(wifiSsid));
  configManagerCopyHotspotSsid(apSsid, sizeof(apSsid));

  const char* mode = (WiFi.getMode() == WIFI_AP) ? "AP" : "STA";

  char json[720];
  snprintf(json, sizeof(json),
           "{\"delaySeconds\":%ld,\"volume\":%ld,\"preloadedIndex\":%u,\"customIndex\":%u,"
           "\"effectiveIndex\":%u,\"wifiConfigured\":%s,\"wifiSsid\":\"%s\",\"apSsid\":\"%s\","
           "\"mode\":\"%s\",\"ip\":\"%s\",\"connected\":%s,\"connectedSsid\":\"%s\",\"rssi\":%ld}",
           static_cast<long>(configManagerGetDelaySeconds()), static_cast<long>(configManagerGetVolume()),
           static_cast<unsigned>(configManagerGetPreloadedTrackIndex()),
           static_cast<unsigned>(configManagerGetCustomTrackIndex()),
           static_cast<unsigned>(configManagerGetEffectiveTrackIndex()),
           configManagerHasWifiCredentials() ? "true" : "false", wifiSsid, apSsid, mode, ipBuf,
           connected ? "true" : "false", connectedSsid.c_str(), static_cast<long>(rssi));
  g_server.send(200, "application/json", json);
}

void handleWifiScan() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  (void)eventBusSend(EVENT_WIFI_SCAN, 0);
  char json[1600];
  networkManagerCopyScanJson(json, sizeof(json));
  g_server.send(200, "application/json", json);
}

void handleWifiConnect() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  if (!g_server.hasArg("ssid") || !g_server.hasArg("password")) {
    g_server.send(400, "text/plain", "Missing ssid/password");
    return;
  }
  const String ssid = g_server.arg("ssid");
  const String password = g_server.arg("password");
  (void)eventBusSendText(EVENT_WIFI_SET, 0, ssid.c_str(), password.c_str());
  (void)eventBusSend(EVENT_SYSTEM_RESTART, 1500);
  g_server.send(200, "text/plain", "WiFi saved; restarting...");
}

void handleWifiForget() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  (void)eventBusSend(EVENT_WIFI_FORGET, 0);
  (void)eventBusSend(EVENT_SYSTEM_RESTART, 1500);
  g_server.send(200, "text/plain", "WiFi cleared; restarting...");
}

void handleHotspotSet() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  if (!g_server.hasArg("ssid") || !g_server.hasArg("password")) {
    g_server.send(400, "text/plain", "Missing ssid/password");
    return;
  }
  const String ssid = g_server.arg("ssid");
  const String password = g_server.arg("password");
  if (password.length() < 8) {
    g_server.send(400, "text/plain", "Password must be >= 8 chars");
    return;
  }
  (void)eventBusSendText(EVENT_HOTSPOT_SET, 0, ssid.c_str(), password.c_str());
  (void)eventBusSend(EVENT_SYSTEM_RESTART, 1500);
  g_server.send(200, "text/plain", "Hotspot saved; restarting...");
}

void handleAudioSet() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  if (!g_server.hasArg("preloaded") || !g_server.hasArg("custom")) {
    g_server.send(400, "text/plain", "Missing preloaded/custom");
    return;
  }
  const long pre = g_server.arg("preloaded").toInt();
  const long custom = g_server.arg("custom").toInt();
  (void)eventBusSend(EVENT_AUDIO_SET_PRELOADED, static_cast<int32_t>(pre));
  (void)eventBusSend(EVENT_AUDIO_SET_CUSTOM, static_cast<int32_t>(custom));
  if (g_server.hasArg("volume")) {
    const long vol = g_server.arg("volume").toInt();
    (void)eventBusSend(EVENT_VOLUME_CHANGE, static_cast<int32_t>(vol));
  }
  g_server.send(200, "text/plain", "Audio settings saved");
}

void handleRestart() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  (void)eventBusSend(EVENT_SYSTEM_RESTART, 500);
  g_server.send(200, "text/plain", "Restarting...");
}

void handleFactoryReset() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  if (!g_server.hasArg("confirm") || g_server.arg("confirm") != "YES") {
    g_server.send(400, "text/plain", "Missing confirm=YES");
    return;
  }
  (void)eventBusSend(EVENT_SYSTEM_FACTORY_RESET, 500);
  g_server.send(200, "text/plain", "Factory reset; restarting...");
}

void handleBackup() {
  if (!authManagerEnsure(g_server)) {
    return;
  }

  char wifiSsid[33];
  char wifiPass[65];
  char apSsid[33];
  char apPass[65];
  configManagerCopyWifiCredentials(wifiSsid, sizeof(wifiSsid), wifiPass, sizeof(wifiPass));
  configManagerCopyHotspotCredentials(apSsid, sizeof(apSsid), apPass, sizeof(apPass));

  char out[1024];
  snprintf(out, sizeof(out),
           "delaySeconds=%ld\nvolume=%ld\npreloadedIndex=%u\ncustomIndex=%u\nwifiSsid=%s\nwifiPassword=%s\napSsid=%s\napPassword=%s",
           static_cast<long>(configManagerGetDelaySeconds()), static_cast<long>(configManagerGetVolume()),
           static_cast<unsigned>(configManagerGetPreloadedTrackIndex()),
           static_cast<unsigned>(configManagerGetCustomTrackIndex()),
           wifiSsid, wifiPass, apSsid, apPass);

  g_server.send(200, "text/plain", out);
}

void handleRestore() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  const String body = g_server.arg("plain");
  if (body.length() == 0) {
    g_server.send(400, "text/plain", "Missing body");
    return;
  }

  // Very small key=value parser (newline separated).
  int32_t delay = -1;
  int32_t vol = -1;
  int32_t pre = -1;
  int32_t custom = -1;
  String wifiSsid;
  String wifiPassword;
  String apSsid;
  String apPassword;

  int pos = 0;
  while (pos < body.length()) {
    int end = body.indexOf('\n', pos);
    if (end < 0) {
      end = body.length();
    }
    const String line = body.substring(pos, end);
    const int eq = line.indexOf('=');
    if (eq > 0) {
      const String key = line.substring(0, eq);
      const String val = line.substring(eq + 1);
      if (key == "delaySeconds") {
        delay = val.toInt();
      } else if (key == "volume") {
        vol = val.toInt();
      } else if (key == "preloadedIndex") {
        pre = val.toInt();
      } else if (key == "customIndex") {
        custom = val.toInt();
      } else if (key == "wifiSsid") {
        wifiSsid = val;
      } else if (key == "wifiPassword") {
        wifiPassword = val;
      } else if (key == "apSsid") {
        apSsid = val;
      } else if (key == "apPassword") {
        apPassword = val;
      }
    }
    pos = end + 1;
  }

  if (delay >= 0) {
    (void)eventBusSend(EVENT_SET_DELAY, delay);
  }
  if (vol >= 0) {
    (void)eventBusSend(EVENT_VOLUME_CHANGE, vol);
  }
  if (pre >= 0) {
    (void)eventBusSend(EVENT_AUDIO_SET_PRELOADED, pre);
  }
  if (custom >= 0) {
    (void)eventBusSend(EVENT_AUDIO_SET_CUSTOM, custom);
  }
  if (wifiSsid.length() > 0 && wifiPassword.length() > 0) {
    (void)eventBusSendText(EVENT_WIFI_SET, 0, wifiSsid.c_str(), wifiPassword.c_str());
  }
  if (apSsid.length() > 0 && apPassword.length() > 0) {
    (void)eventBusSendText(EVENT_HOTSPOT_SET, 0, apSsid.c_str(), apPassword.c_str());
  }

  (void)eventBusSend(EVENT_SYSTEM_RESTART, 1500);
  g_server.send(200, "text/plain", "Restored; restarting...");
}

void handleTimeSync() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  if (!g_server.hasArg("epochMs")) {
    g_server.send(400, "text/plain", "Missing epochMs");
    return;
  }
  const String epochMs = g_server.arg("epochMs");
  (void)eventBusSendText(EVENT_TIME_SYNC, 0, epochMs.c_str(), nullptr);
  g_server.send(200, "text/plain", "Time synced");
}

void handleSystemInfo() {
  if (!authManagerEnsure(g_server)) {
    return;
  }

  // === HARDWARE INFO ===
  esp_chip_info_t chipInfo;
  esp_chip_info(&chipInfo);
  
  const char* chipModel = "Unknown";
  uint32_t cpuFreq = 80;  // Default fallback
  switch (chipInfo.model) {
    case CHIP_ESP32:
      chipModel = "ESP32 (Xtensa LX6)";
      cpuFreq = 240;
      break;
    case CHIP_ESP32S3:
      chipModel = "ESP32-S3 (Xtensa LX7)";
      cpuFreq = 240;
      break;
    case CHIP_ESP32C3:
      chipModel = "ESP32-C3 (RISC-V)";
      cpuFreq = 160;
      break;
    case CHIP_ESP32S2:
      chipModel = "ESP32-S2 (Xtensa LX7)";
      cpuFreq = 240;
      break;
    default:
      chipModel = "Unknown";
      break;
  }

  // === MEMORY INFO ===
  const uint32_t heapFree = esp_get_free_heap_size();
  const uint32_t heapMax = 320 * 1024;
  const uint32_t heapUsed = heapMax - heapFree;
  const uint32_t heapPercent = (heapUsed * 100) / heapMax;
  const uint32_t minHeapFree = esp_get_minimum_free_heap_size();

  // === FLASH INFO ===
  uint32_t flashSize = 0;
  esp_flash_get_size(NULL, (uint32_t*)&flashSize);
  const uint32_t flashSizeKB = flashSize / 1024;

  // === MAC ADDRESS ===
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // === UPTIME & TIMESTAMPS ===
  const uint32_t uptime = millis() / 1000;
  const uint32_t uptimeDays = uptime / 86400;
  const uint32_t uptimeHours = (uptime % 86400) / 3600;
  const uint32_t uptimeMinutes = (uptime % 3600) / 60;
  const uint32_t uptimeSeconds = uptime % 60;

  // === WIFI INFO ===
  const bool connected = (WiFi.status() == WL_CONNECTED);
  const int32_t rssi = connected ? static_cast<int32_t>(WiFi.RSSI()) : 0;
  const char* wifiMode = (WiFi.getMode() == WIFI_AP) ? "AP" : "STA";
  
  const IPAddress ip = (WiFi.getMode() == WIFI_AP) ? WiFi.softAPIP() : WiFi.localIP();
  char ipStr[16];
  snprintf(ipStr, sizeof(ipStr), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  
  const IPAddress gateway = WiFi.gatewayIP();
  char gwStr[16];
  snprintf(gwStr, sizeof(gwStr), "%u.%u.%u.%u", gateway[0], gateway[1], gateway[2], gateway[3]);
  
  const IPAddress subnet = WiFi.subnetMask();
  char subnetStr[16];
  snprintf(subnetStr, sizeof(subnetStr), "%u.%u.%u.%u", subnet[0], subnet[1], subnet[2], subnet[3]);

  // === RUNTIME INFO ===
  uint32_t taskCount = uxTaskGetNumberOfTasks();

  // === BUILD COMPREHENSIVE JSON ===
  char json[3200];
  snprintf(json, sizeof(json),
           "{"
           "\"hardware\":{"
           "\"chip\":\"%s\","
           "\"cores\":%u,"
           "\"revision\":\"0x%02x_%02x\","
           "\"cpuFreq\":%u,"
           "\"features\":\"WiFi%s%s\""
           "},"
           "\"memory\":{"
           "\"heapTotal\":%lu,"
           "\"heapFree\":%lu,"
           "\"heapUsed\":%lu,"
           "\"heapPercent\":%u,"
           "\"minHeapFree\":%lu,"
           "\"heapFragmented\":0"
           "},"
           "\"flash\":{"
           "\"size\":%lu,"
           "\"available\":0"
           "},"
           "\"network\":{"
           "\"wifiMode\":\"%s\","
           "\"connected\":%s,"
           "\"rssi\":%ld,"
           "\"mac\":\"%s\","
           "\"ip\":\"%s\","
           "\"gateway\":\"%s\","
           "\"subnet\":\"%s\","
           "\"dns1\":\"0.0.0.0\","
           "\"dns2\":\"0.0.0.0\""
           "},"
           "\"system\":{"
           "\"uptime\":\"P%uDT%02uH%02uM%02uS\","
           "\"uptimeSeconds\":%lu,"
           "\"taskCount\":%lu,"
           "\"bootCount\":0"
           "}"
           "}",
           chipModel,
           static_cast<unsigned>(chipInfo.cores),
           static_cast<unsigned>((chipInfo.revision >> 8) & 0xFF),
           static_cast<unsigned>(chipInfo.revision & 0xFF),
           cpuFreq,
           (chipInfo.features & 1) ? ",BLE" : "",
           (chipInfo.features & 4) ? ",BT" : "",
           static_cast<unsigned long>(heapMax),
           static_cast<unsigned long>(heapFree),
           static_cast<unsigned long>(heapUsed),
           static_cast<unsigned>(heapPercent),
           static_cast<unsigned long>(minHeapFree),
           static_cast<unsigned long>(flashSizeKB),
           wifiMode,
           connected ? "true" : "false",
           static_cast<long>(rssi),
           macStr,
           ipStr,
           gwStr,
           subnetStr,
           static_cast<unsigned>(uptimeDays),
           static_cast<unsigned>(uptimeHours),
           static_cast<unsigned>(uptimeMinutes),
           static_cast<unsigned>(uptimeSeconds),
           static_cast<unsigned long>(uptime),
           static_cast<unsigned long>(taskCount));
  
  g_server.send(200, "application/json", json);
}

void handleSystemDetailsPage() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  g_server.send_P(200, "text/html", kSystemDetailsHtml);
}

void handleLogs() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  char buf[2200];
  loggerCopyRecent(buf, sizeof(buf));
  g_server.send(200, "text/plain", buf);
}

void webTask(void*) {
  for (;;) {
    if (g_serverStarted) {
      g_server.handleClient();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

}

void webServerInit() {
  g_server.on("/", HTTP_GET, handleRoot);
  g_server.on("/logview", HTTP_GET, handleLogView);
  g_server.on("/sysinfo", HTTP_GET, handleSystemDetailsPage);
  g_server.on("/play", HTTP_GET, handlePlay);
  g_server.on("/setDelay", HTTP_GET, handleSetDelay);
  g_server.on("/logs", HTTP_GET, handleLogs);
  g_server.on("/api/settings", HTTP_GET, handleSettings);
  g_server.on("/api/sysinfo", HTTP_GET, handleSystemInfo);
  g_server.on("/api/wifi/scan", HTTP_GET, handleWifiScan);
  g_server.on("/api/wifi/connect", HTTP_POST, handleWifiConnect);
  g_server.on("/api/wifi/forget", HTTP_POST, handleWifiForget);
  g_server.on("/api/hotspot", HTTP_POST, handleHotspotSet);
  g_server.on("/api/audio", HTTP_POST, handleAudioSet);
  g_server.on("/api/restart", HTTP_POST, handleRestart);
  g_server.on("/api/factory_reset", HTTP_POST, handleFactoryReset);
  g_server.on("/api/backup", HTTP_GET, handleBackup);
  g_server.on("/api/restore", HTTP_POST, handleRestore);
  g_server.on("/api/time/sync", HTTP_POST, handleTimeSync);
}

void webServerStartTask(UBaseType_t priority, uint32_t stackWords) {
  if (!g_serverStarted) {
    g_server.begin();
    g_serverStarted = true;
    logInfo("WEB", "Server started");
  }
  if (g_webTask != nullptr) {
    return;
  }
  const BaseType_t ok = xTaskCreate(webTask, "web", stackWords, nullptr, priority, &g_webTask);
  if (ok != pdPASS) {
    g_webTask = nullptr;
    logError("WEB", "Failed to start web task");
  }
}
