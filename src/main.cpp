#ifndef NATIVE_BUILD

#include <Arduino.h>
#include "display.h"
#include "render.h"
#include "ticker.h"
#include "data_provider.h"

#ifdef USE_FPVGATE
#include "wifi_manager.h"
#include "fpvgate_provider.h"
#else
#include "simulation.h"
#endif

LGFX lcd;
render::RenderState render_state;
ticker::TickerState ticker_state;

#ifdef USE_FPVGATE
FPVGateProvider fpvgate;
DataProvider* provider = &fpvgate;
#else
SimulationEngine simulation;
DataProvider* provider = &simulation;
#endif

void setup() {
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && (millis() - start < 3000)) { delay(10); }
    Serial.println();
    Serial.println("RaceGate Hub starting...");

    if (!display::init_display(lcd)) {
        Serial.println("ERROR: Display init failed!");
        display::signal_error();
        return;
    }

    display::show_splash_screen(lcd);
    delay(2000);

    lcd.fillScreen(TFT_BLACK);

#ifdef USE_FPVGATE
    // Show WiFi connecting message on screen BEFORE blocking autoConnect
    lcd.setTextDatum(middle_center);
    lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    lcd.setFont(&fonts::Font4);
    lcd.drawString("Conectando WiFi...", 240, 100);
    lcd.setFont(&fonts::Font2);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.drawString("Si no hay red guardada:", 240, 150);
    lcd.setTextColor(0x07FF, TFT_BLACK);  // Cyan
    lcd.drawString("1. Conectate desde celular a:", 240, 180);
    lcd.setFont(&fonts::Font4);
    lcd.drawString("RaceGate_Display", 240, 210);
    lcd.setFont(&fonts::Font2);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.drawString("Password: racegate1", 240, 245);
    lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
    lcd.drawString("2. Elegir red WiFi en el portal", 240, 275);

    // Now call blocking WiFiManager
    auto wifi_result = wifi_setup::connect_wifi();

    if (!wifi_result.connected) {
        Serial.println("WiFi no conectado. Reiniciando...");
        lcd.fillScreen(TFT_BLACK);
        lcd.setTextDatum(middle_center);
        lcd.setTextColor(TFT_RED, TFT_BLACK);
        lcd.setFont(&fonts::Font4);
        lcd.drawString("WiFi TIMEOUT", 240, 140);
        lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        lcd.setFont(&fonts::Font2);
        lcd.drawString("Reiniciando en 5s...", 240, 180);
        delay(5000);
        ESP.restart();
    }

    // WiFi connected - show success
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextDatum(middle_center);
    lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    lcd.setFont(&fonts::Font4);
    lcd.drawString("WiFi OK!", 240, 120);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setFont(&fonts::Font2);
    char buf[40];
    snprintf(buf, sizeof(buf), "Red: %s", wifi_result.ssid);
    lcd.drawString(buf, 240, 160);
    snprintf(buf, sizeof(buf), "IP: %s", wifi_result.ip);
    lcd.drawString(buf, 240, 190);
    lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    lcd.drawString("Configurar webhook a esta IP", 240, 230);
    delay(4000);

    if (!fpvgate.init()) {
        Serial.println("ERROR: FPVGate server init failed!");
    }
#else
    simulation.init(6);
#endif

    lcd.fillScreen(TFT_BLACK);
    render::init_render(render_state);
    render::draw_header(lcd, render_state);
    ticker::init_ticker(ticker_state);

    Serial.println("Init complete. Dashboard running.");
}

void loop() {
    provider->update();

    if (provider->has_new_data()) {
        PilotEntry pilots[MAX_PILOTS];
        uint8_t count = 0;

#ifdef USE_FPVGATE
        // Single-pilot mode: show lap history instead of leaderboard
        if (fpvgate.is_single_pilot()) {
            count = fpvgate.get_lap_history(pilots, MAX_PILOTS);
        } else {
            count = provider->get_pilots(pilots, MAX_PILOTS);
        }
#else
        count = provider->get_pilots(pilots, MAX_PILOTS);
#endif

        if (count > 0) {
            render::render_dashboard(lcd, render_state, pilots, count);
            ticker::generate_message(ticker_state, pilots, count);
        }
    }

    // Always update ticker animation
    ticker::render_ticker(lcd, ticker_state, millis());
}

#endif // NATIVE_BUILD
