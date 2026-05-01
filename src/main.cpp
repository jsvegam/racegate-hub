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

static bool display_ok = false;

// Set to false to run in serial-only mode (no TFT required)
// Set to true when TFT display is physically connected
static const bool TFT_CONNECTED = true;

void setup() {
    Serial.begin(115200);
    // Wait for USB CDC serial to be ready (ESP32-S3 native USB)
    unsigned long start = millis();
    while (!Serial && (millis() - start < 3000)) {
        delay(10);
    }
    Serial.println();
    Serial.println("RaceGate Hub starting...");

    if (TFT_CONNECTED) {
        Serial.println("Initializing TFT display...");
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

        // Initialize render state and draw header (Req 2.2)
        render::init_render(render_state);
        render::draw_header(tft, render_state);
        display_ok = true;
        Serial.println("TFT display initialized.");
    } else {
        Serial.println("TFT_CONNECTED=false, running in serial-only mode.");
    }

    // Initialize simulation with 6 pilots (Req 4.1)
    simulation.init(6);
    Serial.println("Simulation initialized with 6 pilots.");
    Serial.println("Init complete. Dashboard running.");
}

void loop() {
    // Update simulation (Req 4.3)
    simulation.update();

    // Only render/print when there's new data (Req 5.1, 6.4)
    if (simulation.has_new_data()) {
        PilotEntry pilots[MAX_PILOTS];
        uint8_t count = simulation.get_pilots(pilots, MAX_PILOTS);

        // Render to TFT if available
        if (display_ok) {
            render::render_dashboard(tft, render_state, pilots, count);
        }

        // Always print to serial for debugging
        Serial.println("--- Race Status ---");
        for (uint8_t i = 0; i < count; ++i) {
            char line[80];
            char last_buf[16], best_buf[16], gap_buf[16];
            render::format_time(pilots[i].last_lap_ms, last_buf, sizeof(last_buf));
            render::format_time(pilots[i].best_lap_ms, best_buf, sizeof(best_buf));
            render::format_gap(pilots[i].gap_ms, gap_buf, sizeof(gap_buf));
            snprintf(line, sizeof(line), "P%d %-8s Last:%s Best:%s Laps:%d Gap:%s",
                     pilots[i].position, pilots[i].name,
                     last_buf, best_buf, pilots[i].lap_count, gap_buf);
            Serial.println(line);
        }
    }
}

#endif // NATIVE_BUILD
