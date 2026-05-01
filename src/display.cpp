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
    digitalWrite(BL_PIN, LOW);  // Backlight off during init

    // Configure SPI pins explicitly before TFT init
    // This prevents crashes when SPI bus is not properly initialized
    pinMode(10, OUTPUT);  // MOSI
    pinMode(9, INPUT);    // MISO
    pinMode(8, OUTPUT);   // SCK
    pinMode(2, OUTPUT);   // CS
    pinMode(4, OUTPUT);   // DC
    pinMode(5, OUTPUT);   // RST

    // Pull CS high (deselect) before init
    digitalWrite(2, HIGH);

    // Hardware reset the TFT
    digitalWrite(5, LOW);
    delay(50);
    digitalWrite(5, HIGH);
    delay(150);

    // Initialize TFT
    tft.init();
    tft.setRotation(1);  // Landscape 480x320
    tft.fillScreen(TFT_BLACK);

    // Turn on backlight after successful init
    digitalWrite(BL_PIN, HIGH);

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
