#pragma once

#include <Arduino.h>

void configManagerInit();
void configManagerStartTask(UBaseType_t priority = 1, uint32_t stackWords = 4096);

bool configManagerHasWifiCredentials();
void configManagerCopyWifiCredentials(char* ssidOut, size_t ssidOutSize, char* passwordOut, size_t passwordOutSize);

bool configManagerHasHotspotCredentials();
void configManagerCopyHotspotCredentials(char* ssidOut,
                                        size_t ssidOutSize,
                                        char* passwordOut,
                                        size_t passwordOutSize);
void configManagerEnsureHotspotCredentials(const char* defaultSsid, const char* defaultPassword);
