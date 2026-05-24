# Changelog

All notable changes to RaceGate Hub are documented here.

## [0.3.0] - 2026-05-25

### Added
- **WiFi Manager**: Captive portal for WiFi configuration from phone — no recompilation needed to change networks
- **FPVGate integration**: Real-time webhook receiver for live race data
- **Smart display mode**: Automatically detects single-pilot vs multi-pilot
  - Single pilot: Shows lap-by-lap history with gap vs personal best
  - Multi pilot: Shows leaderboard with positions and live sorting
- **Network architecture documentation**: Full explanation of WiFi topology and design decisions
- New build environment `esp32s3_fpvgate` for FPVGate integration mode

### Changed
- Wiring guide corrected with accurate GPIO numbers matching Seeed XIAO ESP32-S3 official pinout
- Resolved webhook blocker: FPVGate webhooks require all devices on same external WiFi network (not FPVGate's own AP)
- Updated README to professional English documentation with FAQ section

### Technical Details
- WiFi Manager library: [tzapu/WiFiManager](https://github.com/tzapu/WiFiManager)
- Credentials stored in ESP32 NVS flash (persist across reboots)
- Portal timeout: 10 minutes
- `setCleanConnect(true)` fixes "sta is connecting" errors on ESP32-S3

---

## [0.2.0] - 2026-05-24

### Added
- **FPVGateProvider** class implementing DataProvider interface for webhook reception
- Network architecture diagrams for multi-node FPVGate setup
- Timer wiring documentation (XIAO ESP32-S3 → RX5808)
- FPVGate flash instructions (web flasher + PlatformIO)
- Hardware inventory with purchase links and costs

### Changed
- Updated wiring guide with color-coded cable diagrams for both devices

---

## [0.1.0] - 2026-05-23

### Added
- **Display standalone with simulation engine**
- Terminal-style dark theme with race palette (cyan headers, amber names, green leader, red worst)
- Gap delta coloring (green = improving, red = falling behind)
- Race commentary ticker with particle explosion effects
- Partial-update rendering engine (flicker-free, 5-10 FPS)
- LovyanGFX driver for ILI9488 on XIAO ESP32-S3
- DataProvider abstract interface for data source decoupling
- SimulationEngine with 6 pilots, realistic lap times (10-20s range)
- Race logic: sorting by laps then time, gap calculation
- Unit tests with GoogleTest + RapidCheck (64 test cases)
- PlatformIO project with 3 environments (esp32s3, esp32s3_fpvgate, native)

---

## Version History Summary

| Version | Milestone |
|---------|-----------|
| 0.1.0 | Display working with simulation |
| 0.2.0 | FPVGate provider + hardware docs |
| 0.3.0 | **End-to-end integration working** (WiFi Manager + webhooks + smart display) |
