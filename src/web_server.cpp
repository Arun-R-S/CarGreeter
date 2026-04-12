#include "web_server.h"

#include <WebServer.h>

#include "auth_manager.h"
#include "event_bus.h"
#include "file_manager.h"
#include "logger.h"
#include "web_pages.h"

namespace {

WebServer g_server(80);
TaskHandle_t g_webTask = nullptr;
bool g_uploadOk = false;

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

void handleLogs() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  char buf[1600];
  loggerCopyRecent(buf, sizeof(buf));
  g_server.send(200, "text/plain", buf);
}

void handleUploadDone() {
  if (!authManagerEnsure(g_server)) {
    return;
  }
  if (g_uploadOk) {
    (void)eventBusSend(EVENT_UPLOAD_DONE, 0);
    g_server.send(200, "text/plain", "Upload successful");
    logInfo("WEB", "EVENT_UPLOAD_DONE sent");
  } else {
    g_server.send(500, "text/plain", "Upload failed");
  }
}

void handleUploadStream() {
  if (!authManagerEnsure(g_server)) {
    return;
  }

  HTTPUpload& upload = g_server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    g_uploadOk = fileManagerBeginUpload(fileManagerAudioPath());
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (g_uploadOk) {
      g_uploadOk = fileManagerWriteUpload(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    g_uploadOk = fileManagerEndUpload(g_uploadOk);
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    (void)fileManagerEndUpload(false);
    g_uploadOk = false;
  }
}

void webTask(void*) {
  for (;;) {
    g_server.handleClient();
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
  g_server.on("/upload", HTTP_POST, handleUploadDone, handleUploadStream);
  g_server.begin();
  logInfo("WEB", "Server started");
}

void webServerStartTask(UBaseType_t priority, uint32_t stackWords) {
  if (g_webTask != nullptr) {
    return;
  }
  const BaseType_t ok = xTaskCreate(webTask, "web", stackWords, nullptr, priority, &g_webTask);
  if (ok != pdPASS) {
    g_webTask = nullptr;
    logError("WEB", "Failed to start web task");
  }
}
