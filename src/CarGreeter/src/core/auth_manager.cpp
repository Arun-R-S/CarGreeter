#include "auth_manager.h"

#include <string.h>

#include "logger.h"

namespace {

char g_username[33] = "admin";
char g_password[65] = "1234";

}

void authManagerInit(const char* username, const char* password) {
  snprintf(g_username, sizeof(g_username), "%s", (username != nullptr) ? username : "admin");
  snprintf(g_password, sizeof(g_password), "%s", (password != nullptr) ? password : "1234");
  logInfo("AUTH", "Auth configured");
}

bool authManagerEnsure(WebServer& server) {
  if (server.authenticate(g_username, g_password)) {
    return true;
  }
  server.requestAuthentication();
  logWarn("AUTH", "Unauthorized request");
  return false;
}
