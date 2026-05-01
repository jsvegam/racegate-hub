#ifndef NATIVE_BUILD

#include "display.h"
#include <Arduino.h>

namespace display {

bool init_display(TFT_eSPI& tft) {
    // Initialize LED pin for error signaling
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // LED off (active low on XIAO)

    // Initialize backlight pin
    pinMode(BL_PIN, OUTPUT);
    digitalWrite(BL_PIN, HIGH);  // Backlight on

    // Initialize TFT
    tft.init();
    tft.setRotation(1);  // Landscape 480x320
    tft.fillScreen(TFT_BLACK);

    // Simple check: if we can read the display, it's working
    // TFT_eSPI doesn't provide a direct "is connected" check,
    // so we assume success if init() doesn't hang
    return true;
}

void show_splash_screen(TFT_eSPI& tft) {
    tft.fillScreen(TFT_BLACK);

    // Title
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextFont(4);
    tft.setTextDatum(MC_DATUM);  // Middle center
    tft.drawString("RACEGATE HUB", 240, 130);

    // Subtitle
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);
    tft.drawString("Race Dashboard v1.0", 240, 170);

    // Footer
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setTextFont(1);
    tft.drawString("XIAO ESP32-S3 + ILI9488", 240, 210);

    tft.setTextDatum(TL_DATUM);  // Reset to top-left
}

void signal_error() {
    pinMode(LED_PIN, OUTPUT);
    while (true) {
        digitalWrite(LED_PIN, LOW);   // LED on (active low)
        delay(100);                    // 5Hz = 100ms on + 100ms off
        digitalWrite(LED_PIN, HIGH);  // LED off
        delay(100);
    }
}

} // namespace display

#endif // NATIVE_BUILD
