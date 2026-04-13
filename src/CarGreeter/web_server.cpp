#include "web_server.h"

#include <WebServer.h>
#include <WiFi.h>

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
  const bool secrets = g_server.hasArg("secrets") && g_server.arg("secrets") == "1";

  char wifiSsid[33];
  char wifiPass[65];
  char apSsid[33];
  char apPass[65];
  configManagerCopyWifiCredentials(wifiSsid, sizeof(wifiSsid), wifiPass, sizeof(wifiPass));
  configManagerCopyHotspotCredentials(apSsid, sizeof(apSsid), apPass, sizeof(apPass));

  char out[768];
  snprintf(out, sizeof(out),
           "delaySeconds=%ld\nvolume=%ld\npreloadedIndex=%u\ncustomIndex=%u\nwifiSsid=%s\n%s%sapSsid=%s\n%s%s",
           static_cast<long>(configManagerGetDelaySeconds()), static_cast<long>(configManagerGetVolume()),
           static_cast<unsigned>(configManagerGetPreloadedTrackIndex()),
           static_cast<unsigned>(configManagerGetCustomTrackIndex()), wifiSsid,
           secrets ? "wifiPassword=" : "", secrets ? wifiPass : "", secrets ? "\n" : "",
           apSsid, secrets ? "apPassword=" : "", secrets ? apPass : "");

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

void handleLogs() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  char buf[1600];
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
  g_server.on("/play", HTTP_GET, handlePlay);
  g_server.on("/setDelay", HTTP_GET, handleSetDelay);
  g_server.on("/logs", HTTP_GET, handleLogs);
  g_server.on("/api/settings", HTTP_GET, handleSettings);
  g_server.on("/api/wifi/scan", HTTP_GET, handleWifiScan);
  g_server.on("/api/wifi/connect", HTTP_POST, handleWifiConnect);
  g_server.on("/api/wifi/forget", HTTP_POST, handleWifiForget);
  g_server.on("/api/hotspot", HTTP_POST, handleHotspotSet);
  g_server.on("/api/audio", HTTP_POST, handleAudioSet);
  g_server.on("/api/restart", HTTP_POST, handleRestart);
  g_server.on("/api/factory_reset", HTTP_POST, handleFactoryReset);
  g_server.on("/api/backup", HTTP_GET, handleBackup);
  g_server.on("/api/restore", HTTP_POST, handleRestore);
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
