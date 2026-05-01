#ifndef NATIVE_BUILD

#include <Arduino.h>
#include "display.h"
#include "simulation.h"
#include "render.h"

LGFX lcd;
SimulationEngine simulation;
render::RenderState render_state;

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

    simulation.init(6);
    render::init_render(render_state);
    render::draw_header(lcd, render_state);

    Serial.println("Init complete. Dashboard running.");
}

void loop() {
    simulation.update();

    if (simulation.has_new_data()) {
        PilotEntry pilots[MAX_PILOTS];
        uint8_t count = simulation.get_pilots(pilots, MAX_PILOTS);
        render::render_dashboard(lcd, render_state, pilots, count);
    }
}

#endif // NATIVE_BUILD
