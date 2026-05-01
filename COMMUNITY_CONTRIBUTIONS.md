# Community Contributions

Solutions to common issues found during the development of RaceGate Hub that may help others in the community.

---

## 1. TFT_eSPI + ESP32-S3 + ILI9488 — Core Panic (StoreProhibited)

**Forum:** https://github.com/Bodmer/TFT_eSPI/discussions/3392

**Message to post:**

> I had the exact same issue — ESP32-S3 (Seeed Studio XIAO ESP32-S3) + ILI9488 3.5" SPI causing `StoreProhibited` panic on `tft.init()` / `tft.begin()`. Tried multiple SPI frequencies, different SPI hosts (SPI2_HOST, SPI3_HOST), manual pin configuration before init — nothing worked. The crash always happened inside TFT_eSPI's SPI initialization.
>
> **Solution:** Switch to [LovyanGFX](https://github.com/lovyan03/LovyanGFX). It works perfectly with ESP32-S3 + ILI9488 via SPI, no crashes, no workarounds needed.
>
> Here's the LovyanGFX configuration that works for XIAO ESP32-S3 + ILI9488:
>
> ```cpp
> class LGFX : public lgfx::LGFX_Device {
>     lgfx::Panel_ILI9488 _panel_instance;
>     lgfx::Bus_SPI _bus_instance;
>     lgfx::Light_PWM _light_instance;
> public:
>     LGFX(void) {
>         auto cfg = _bus_instance.config();
>         cfg.spi_host = SPI3_HOST;
>         cfg.freq_write = 27000000;
>         cfg.freq_read  = 16000000;
>         cfg.pin_sclk = 7;   // D8 = GPIO7
>         cfg.pin_mosi = 9;   // D10 = GPIO9
>         cfg.pin_miso = 8;   // D9 = GPIO8
>         cfg.pin_dc   = 3;   // D2 = GPIO3
>         _bus_instance.config(cfg);
>         _panel_instance.setBus(&_bus_instance);
>
>         auto pcfg = _panel_instance.config();
>         pcfg.pin_cs   = 1;   // D0 = GPIO1
>         pcfg.pin_rst  = 4;   // D3 = GPIO4
>         pcfg.panel_width  = 320;
>         pcfg.panel_height = 480;
>         _panel_instance.config(pcfg);
>
>         auto lcfg = _light_instance.config();
>         lcfg.pin_bl = 2;     // D1 = GPIO2
>         _light_instance.config(lcfg);
>         _panel_instance.setLight(&_light_instance);
>         setPanel(&_panel_instance);
>     }
> };
> ```
>
> **Important:** On the XIAO ESP32-S3, the D-pin numbers do NOT map 1:1 to GPIO numbers. You must use the actual GPIO numbers in your driver config (see table below).
>
> Full working project: https://github.com/jsvegam/racegate-hub

---

## 2. XIAO ESP32-S3 + ILI9488 — White Screen Only

**Forum:** https://forum.arduino.cc/t/xiao-esp32s3-und-tft-espi-mit-ili9488/1254104

**Message to post:**

> I had the same problem — white screen with backlight on, no image. Two issues were causing it:
>
> **Issue 1: TFT_eSPI crashes on ESP32-S3.** There's a known bug where `tft.init()` causes a `StoreProhibited` core panic on ESP32-S3. The fix is to use [LovyanGFX](https://github.com/lovyan03/LovyanGFX) instead of TFT_eSPI. LovyanGFX works perfectly with ESP32-S3 + ILI9488.
>
> **Issue 2: Wrong GPIO pin numbers.** On the Seeed Studio XIAO ESP32-S3, the D-pin labels do NOT correspond to GPIO numbers. If you use `D8` thinking it's `GPIO8`, you're actually talking to the wrong pin. Here's the correct mapping:
>
> | Board Pin | Actual GPIO |
> |-----------|-------------|
> | D0 | GPIO1 |
> | D1 | GPIO2 |
> | D2 | GPIO3 |
> | D3 | GPIO4 |
> | D4 | GPIO5 |
> | D5 | GPIO6 |
> | D6 | GPIO43 |
> | D7 | GPIO44 |
> | D8 | GPIO7 |
> | D9 | GPIO8 |
> | D10 | GPIO9 |
>
> When configuring your display driver, use the GPIO numbers (right column), not the D-pin numbers.
>
> Working project with LovyanGFX + XIAO ESP32-S3 + ILI9488: https://github.com/jsvegam/racegate-hub

---

## 3. ILI9488 3.5" TFT + ESP32-S3 — White Screen

**Forum:** https://forum.arduino.cc/t/tft-3-5-ili9488-esp32-s3-does-not-work-white-screen/1367961

**Message to post:**

> If you're using a Seeed Studio XIAO ESP32-S3, check two things:
>
> 1. **Use LovyanGFX instead of TFT_eSPI.** TFT_eSPI has a known crash bug with ESP32-S3 that causes core panic on init. LovyanGFX works without issues.
>
> 2. **Use correct GPIO numbers.** The XIAO ESP32-S3 D-pin labels don't match GPIO numbers. D8=GPIO7, D9=GPIO8, D10=GPIO9, D0=GPIO1, etc. Use the actual GPIO numbers in your driver configuration.
>
> Working example: https://github.com/jsvegam/racegate-hub
