#ifndef NATIVE_BUILD

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "display.h"
#include "simulation.h"
#include "render.h"

// Global instances
TFT_eSPI tft = TFT_eSPI();
SimulationEngine simulation;
render::RenderState render_state;

static bool init_ok = false;

void setup() {
    Serial.begin(115200);
    Serial.println("RaceGate Hub starting...");

    // Initialize display (Req 1.1, 1.2)
    if (!display::init_display(tft)) {
        Serial.println("ERROR: Display init failed!");
        display::signal_error();  // Never returns
        return;
    }

    // Show splash screen for 2 seconds (Req 1.3)
    display::show_splash_screen(tft);
    delay(2000);

    // Clear screen and prepare dashboard
    tft.fillScreen(TFT_BLACK);

    // Initialize simulation with 6 pilots (Req 4.1)
    simulation.init(6);

    // Initialize render state and draw header (Req 2.2)
    render::init_render(render_state);
    render::draw_header(tft, render_state);

    init_ok = true;
    Serial.println("Init complete. Dashboard running.");
}

void loop() {
    if (!init_ok) return;

    // Update simulation (Req 4.3)
    simulation.update();

    // Only render when there's new data (Req 5.1, 6.4)
    if (simulation.has_new_data()) {
        PilotEntry pilots[MAX_PILOTS];
        uint8_t count = simulation.get_pilots(pilots, MAX_PILOTS);
        render::render_dashboard(tft, render_state, pilots, count);
    }
}

#endif // NATIVE_BUILD
