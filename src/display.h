#pragma once

#ifndef NATIVE_BUILD

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// XIAO ESP32-S3 Pin Mapping (D-number → GPIO):
// D0=GPIO1, D1=GPIO2, D2=GPIO3, D3=GPIO4, D4=GPIO5, D5=GPIO6
// D6=GPIO43, D7=GPIO44, D8=GPIO7, D9=GPIO8, D10=GPIO9

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9488 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;

public:
    LGFX(void) {
        // SPI bus configuration
        auto cfg = _bus_instance.config();
        cfg.spi_host = SPI3_HOST;
        cfg.spi_mode = 0;
        cfg.freq_write = 27000000;
        cfg.freq_read  = 16000000;
        cfg.pin_sclk = 7;   // D8 = GPIO7
        cfg.pin_mosi = 9;   // D10 = GPIO9
        cfg.pin_miso = 8;   // D9 = GPIO8
        cfg.pin_dc   = 3;   // D2 = GPIO3
        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);

        // Panel configuration
        auto pcfg = _panel_instance.config();
        pcfg.pin_cs   = 1;   // D0 = GPIO1
        pcfg.pin_rst  = 4;   // D3 = GPIO4
        pcfg.pin_busy = -1;
        pcfg.panel_width  = 320;
        pcfg.panel_height = 480;
        pcfg.offset_x = 0;
        pcfg.offset_y = 0;
        pcfg.readable = true;
        pcfg.invert = false;
        pcfg.rgb_order = false;
        pcfg.dlen_16bit = false;
        pcfg.bus_shared = true;
        _panel_instance.config(pcfg);

        // Backlight configuration
        auto lcfg = _light_instance.config();
        lcfg.pin_bl = 2;     // D1 = GPIO2
        lcfg.invert = false;
        lcfg.freq = 44100;
        lcfg.pwm_channel = 7;
        _light_instance.config(lcfg);
        _panel_instance.setLight(&_light_instance);

        setPanel(&_panel_instance);
    }
};

namespace display {
    static const uint8_t LED_PIN = 21;

    bool init_display(LGFX& lcd);
    void show_splash_screen(LGFX& lcd);
    void signal_error();
}

#endif // NATIVE_BUILD
