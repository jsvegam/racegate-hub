#ifndef NATIVE_BUILD

#include <Arduino.h>
#include "display.h"
#include "render.h"
#include "ticker.h"
#include "data_provider.h"

#ifdef USE_FPVGATE
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
    // Connect to FPVGate WiFi and start webhook listener
    FPVGateConfig config;
    config.ssid = FPVGATE_SSID;
    config.password = FPVGATE_PASSWORD;
    config.server_port = 80;

    if (!fpvgate.init(config)) {
        Serial.println("ERROR: FPVGate WiFi connection failed!");
        Serial.println("Falling back to simulation mode...");
        // TODO: could fall back to simulation here
    } else {
        Serial.printf("FPVGate connected. Display IP: %s\n",
                      WiFi.localIP().toString().c_str());
    }
#else
    simulation.init(6);
#endif

    render::init_render(render_state);
    render::draw_header(lcd, render_state);
    ticker::init_ticker(ticker_state);

    Serial.println("Init complete. Dashboard running.");
}

void loop() {
    provider->update();

    if (provider->has_new_data()) {
        PilotEntry pilots[MAX_PILOTS];
        uint8_t count = provider->get_pilots(pilots, MAX_PILOTS);
        if (count > 0) {
            render::render_dashboard(lcd, render_state, pilots, count);
            // Generate race commentary
            ticker::generate_message(ticker_state, pilots, count);
        }
    }

    // Always update ticker animation (particles need smooth updates)
    ticker::render_ticker(lcd, ticker_state, millis());
}

#endif // NATIVE_BUILD
