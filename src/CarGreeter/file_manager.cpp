#if 0
#include "file_manager.h"

#include <SPIFFS.h>

#include "logger.h"

namespace {

constexpr const char* kAudioFilePath = "/audio.wav";
File g_uploadFile;

}

bool fileManagerInit() {
  const bool ok = SPIFFS.begin(true);
  if (!ok) {
    logError("FILE", "SPIFFS mount failed");
  } else {
    logInfo("FILE", "SPIFFS mounted");
  }
  return ok;
}

const char* fileManagerAudioPath() { return kAudioFilePath; }

File fileManagerOpenAudioRead() {
  if (!SPIFFS.exists(kAudioFilePath)) {
    logWarn("FILE", "Audio not found");
    return File();
  }
  return SPIFFS.open(kAudioFilePath, FILE_READ);
}

bool fileManagerBeginUpload(const char* path) {
  if (g_uploadFile) {
    g_uploadFile.close();
  }
  g_uploadFile = SPIFFS.open((path != nullptr) ? path : kAudioFilePath, FILE_WRITE);
  if (!g_uploadFile) {
    logError("FILE", "Upload open failed");
    return false;
  }
  logInfo("FILE", "Upload started");
  return true;
}

bool fileManagerWriteUpload(const uint8_t* data, size_t len) {
  if (!g_uploadFile || data == nullptr || len == 0) {
    return false;
  }
  const size_t w = g_uploadFile.write(data, len);
  return w == len;
}

bool fileManagerEndUpload(bool success) {
  if (g_uploadFile) {
    g_uploadFile.close();
  }
  if (success) {
    logInfo("FILE", "Upload finished");
  } else {
    logWarn("FILE", "Upload failed");
  }
  return success;
}

#endif
