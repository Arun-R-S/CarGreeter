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

int32_t configManagerGetDelaySeconds();
int32_t configManagerGetVolume();
uint16_t configManagerGetPreloadedTrackIndex();
uint16_t configManagerGetCustomTrackIndex();
uint16_t configManagerGetEffectiveTrackIndex();
void configManagerCopyWifiSsid(char* out, size_t outSize);
void configManagerCopyHotspotSsid(char* out, size_t outSize);
void configManagerFactoryReset();
