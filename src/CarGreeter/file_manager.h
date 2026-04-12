#pragma once

#include <Arduino.h>
#include <FS.h>

bool fileManagerInit();

const char* fileManagerAudioPath();
File fileManagerOpenAudioRead();

bool fileManagerBeginUpload(const char* path);
bool fileManagerWriteUpload(const uint8_t* data, size_t len);
bool fileManagerEndUpload(bool success);

