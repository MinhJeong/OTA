#ifndef OTA_WEB_UPDATER_H
#define OTA_WEB_UPDATER_H

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>

class OTAWebUpdater {
  private:
    WebServer *server;
    String host;
    String loginUsername;
    String loginPassword;
    String style;
    String loginIndex;
    String serverIndex;
    
    void setupStyle();
    void setupLoginPage();
    void setupServerPage();
    
  public:
    OTAWebUpdater(WebServer *server, const char* host);
    void setup(const char* username = "Admin", const char* password = "123456");
    void handleLogin();
    void handleServerIndex();
    void handleUpdate();
    void registerEndpoints();
};

#endif