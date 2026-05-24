#pragma once

#ifndef NATIVE_BUILD
#ifdef USE_FPVGATE

#include <WiFiManager.h>

namespace wifi_setup {

// Nombre del AP que crea el display cuando no tiene WiFi configurada.
// El usuario se conecta a esta red desde el celular para configurar.
static const char* AP_NAME = "RaceGate_Display";
static const char* AP_PASSWORD = "racegate1";

// Timeout del portal cautivo en segundos (3 minutos).
// Si nadie configura, el display vuelve a modo simulación.
static const uint16_t PORTAL_TIMEOUT_SEC = 600;  // 10 minutos

struct WiFiResult {
    bool connected;
    char ssid[33];
    char ip[16];
};

// Intenta conectarse a WiFi guardada en flash (NVS).
// Si no hay credenciales o falla, levanta portal cautivo.
// Muestra estado en Serial.
// Retorna true si se conectó exitosamente.
WiFiResult connect_wifi();

// Borra credenciales guardadas y reinicia (para cambiar de red).
void reset_wifi();

}  // namespace wifi_setup

#endif // USE_FPVGATE
#endif // NATIVE_BUILD
