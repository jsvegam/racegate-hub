#ifndef NATIVE_BUILD

#include "display.h"
#include <Arduino.h>

namespace display {

bool init_display(LGFX& lcd) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // LED off (active low on XIAO)

    lcd.init();
    lcd.setRotation(1);  // Landscape 480x320
    lcd.fillScreen(TFT_BLACK);
    lcd.setBrightness(255);  // Full backlight

    return true;
}

void show_splash_screen(LGFX& lcd) {
    lcd.fillScreen(TFT_BLACK);

    lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    lcd.setTextSize(1);
    lcd.setFont(&fonts::Font4);
    lcd.setTextDatum(middle_center);
    lcd.drawString("RACEGATE HUB", 240, 130);

    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setFont(&fonts::Font2);
    lcd.drawString("Race Dashboard v1.0", 240, 170);

    lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
    lcd.setFont(&fonts::Font0);
    lcd.drawString("XIAO ESP32-S3 + ILI9488", 240, 210);

    lcd.setTextDatum(top_left);
}

void signal_error() {
    pinMode(LED_PIN, OUTPUT);
    while (true) {
        digitalWrite(LED_PIN, LOW);
        delay(100);
        digitalWrite(LED_PIN, HIGH);
        delay(100);
    }
}

} // namespace display

#endif // NATIVE_BUILD
