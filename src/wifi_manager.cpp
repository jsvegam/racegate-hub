#ifndef NATIVE_BUILD
#ifdef USE_FPVGATE

#include "wifi_manager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

namespace wifi_setup {

WiFiResult connect_wifi() {
    WiFiResult result = {false, "", ""};

    // Clean WiFi state to avoid "sta is connecting" errors
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(200);

    WiFiManager wm;

    wm.setConfigPortalTimeout(PORTAL_TIMEOUT_SEC);
    wm.setConnectTimeout(20);      // 20s per connection attempt
    wm.setConnectRetries(3);
    wm.setCleanConnect(true);      // Disconnect before connect attempt

    Serial.println("[WiFi] Intentando conectar...");
    Serial.println("[WiFi] Si no hay red guardada, se abre portal cautivo:");
    Serial.printf("[WiFi]   AP: %s / Password: %s\n", AP_NAME, AP_PASSWORD);

    bool connected = wm.autoConnect(AP_NAME, AP_PASSWORD);

    if (connected) {
        result.connected = true;
        strncpy(result.ssid, WiFi.SSID().c_str(), 32);
        result.ssid[32] = '\0';
        strncpy(result.ip, WiFi.localIP().toString().c_str(), 15);
        result.ip[15] = '\0';

        Serial.printf("[WiFi] Conectado a: %s\n", result.ssid);
        Serial.printf("[WiFi] IP: %s\n", result.ip);
    } else {
        Serial.println("[WiFi] No se pudo conectar. Timeout del portal.");
    }

    return result;
}

void reset_wifi() {
    WiFiManager wm;
    wm.resetSettings();
    Serial.println("[WiFi] Credenciales borradas. Reiniciando...");
    delay(1000);
    ESP.restart();
}

}  // namespace wifi_setup

#endif // USE_FPVGATE
#endif // NATIVE_BUILD
