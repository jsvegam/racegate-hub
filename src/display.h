#pragma once

#ifndef NATIVE_BUILD

#include <TFT_eSPI.h>

namespace display {

    // LED pin for error signaling (XIAO ESP32-S3 built-in LED)
    static const uint8_t LED_PIN = 21;

    // Backlight pin
    static const uint8_t BL_PIN = 3;

    // Initializes TFT display via SPI, configures rotation for 480x320,
    // turns on backlight. Returns true if successful.
    bool init_display(TFT_eSPI& tft);

    // Shows splash screen with system name centered on display.
    void show_splash_screen(TFT_eSPI& tft);

    // Blinks built-in LED at 5Hz indefinitely to signal error.
    // This function never returns.
    void signal_error();

} // namespace display

#endif // NATIVE_BUILD
