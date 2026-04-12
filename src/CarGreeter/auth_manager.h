#pragma once

#include <Arduino.h>
#include <WebServer.h>

void authManagerInit(const char* username, const char* password);
bool authManagerEnsure(WebServer& server);

