# RaceGate Hub

**Standalone FPV race leaderboard display** powered by a Seeed Studio XIAO ESP32-S3 and a 3.5" ILI9488 TFT (480×320). Receives real-time lap data from [FPVGate](https://github.com/LouisHitchcock/FPVGate) timers via WiFi webhooks and renders a live race dashboard.

## Features

- **Smart display mode**: Automatically detects single-pilot vs multi-pilot races
  - **Single pilot**: Shows lap-by-lap history with times and gap vs personal best
  - **Multi pilot**: Shows leaderboard with positions, gaps, and live sorting
- **WiFi Manager**: Configure WiFi from your phone — no recompilation needed
- **Real-time webhooks**: Receives lap events from FPVGate with zero latency
- **Partial-update rendering**: Flicker-free display at 5–10 FPS
- **Dark theme**: Terminal-style palette optimized for outdoor visibility
- **Simulation mode**: Built-in demo mode for development and showcasing
- **Modular architecture**: `DataProvider` interface decouples data source from rendering

## Quick Start

### Hardware Required

| Component | Model | ~Cost |
|-----------|-------|-------|
| MCU | Seeed Studio XIAO ESP32-S3 | $7 |
| Display | ILI9488 3.5" TFT SPI (480×320) | $12 |
| Timer (separate device) | XIAO ESP32-S3 + RX5808 + FPVGate firmware | $12 |

### Wiring (Display)

| Color | XIAO Pin | TFT Pin | Function |
|-------|----------|---------|----------|
| 🟣 Purple | D10 (GPIO9) | SDI (MOSI) | SPI Data Out |
| ⚪ White | D9 (GPIO8) | SDO (MISO) | SPI Data In |
| 🟤 Brown | D8 (GPIO7) | SCK | SPI Clock |
| 🟡 Yellow | D0 (GPIO1) | CS | Chip Select |
| 🟠 Orange | D1 (GPIO2) | LED | Backlight |
| 🔵 Blue | D2 (GPIO3) | DC/RS | Data/Command |
| 🟢 Green | D3 (GPIO4) | RESET | Reset |
| 🔴 Red | 5V | VCC | Power |
| ⚫ Black | GND | GND | Ground |

> Full wiring diagrams with ASCII art: [docs/WIRING_GUIDE.md](docs/WIRING_GUIDE.md)

### Flash & Run

```bash
# Clone
git clone https://github.com/yourusername/RaceGateHub.git
cd RaceGateHub

# Flash simulation mode (no WiFi needed)
pio run -e esp32s3 -t upload

# Flash FPVGate integration mode (with WiFi Manager)
pio run -e esp32s3_fpvgate -t upload
```

### First-Time WiFi Setup (FPVGate mode)

1. Flash with `esp32s3_fpvgate` environment
2. Display shows: "Conectando WiFi... Connect to: RaceGate_Display"
3. From your phone, connect to WiFi **"RaceGate_Display"** (password: `racegate1`)
4. A captive portal opens — select your WiFi network and enter password
5. Display connects and shows its IP address
6. Configure that IP as the webhook target in FPVGate's web interface

Credentials are saved in flash. Next boot connects automatically.

---

## How It Works

### Network Architecture

All devices connect to the same WiFi network (your home router or a phone hotspot):

```
WiFi Network (home router or phone hotspot)
   │
   ├── FPVGate Timer 1 (STA) ─── detects laps via RSSI
   ├── FPVGate Timer 2 (STA) ─── (optional, for multi-pilot)
   │
   ├── RaceGate Display (STA) ── receives webhooks, shows dashboard
   │
   └── Your phone/PC ─────────── FPVGate web UI + WiFi config
```

FPVGate sends HTTP POST webhooks to the display when events occur:

| Endpoint | Event | Display Action |
|----------|-------|----------------|
| `POST /Lap` | Drone crossed the gate | Record lap, update times |
| `POST /RaceStart` | Race started | Reset all data |
| `POST /RaceStop` | Race ended | Freeze display |

### Smart Display Modes

The display automatically adapts based on how many pilots are detected:

**Single Pilot** (1 timer node sending laps):
```
 LAP │  TIME   │  BEST  │   GAP
─────┼─────────┼────────┼─────────
  1  │  5.833  │ 5.833  │   ---
  2  │  6.146  │ 5.833  │ +0.313
  3  │  7.984  │ 5.833  │ +2.151
  4  │  8.295  │ 5.833  │ +2.462
```

**Multi Pilot** (multiple timer nodes):
```
 POS │  PILOT  │  LAST  │  BEST  │ LAPS │  GAP
─────┼─────────┼────────┼────────┼──────┼─────────
  1  │ RAZOR   │ 12.345 │ 11.234 │  15  │   ---
  2  │ VIPER   │ 13.456 │ 12.100 │  14  │ +1.234
  3  │ GHOST   │ 14.567 │ 12.890 │  14  │ +3.456
```

---

## Project Structure

```
├── src/
│   ├── main.cpp              # Main orchestrator
│   ├── display.h/.cpp        # TFT hardware init, splash screen
│   ├── data_provider.h       # Abstract data interface
│   ├── simulation.h/.cpp     # Demo simulation engine
│   ├── fpvgate_provider.h/.cpp # FPVGate webhook receiver
│   ├── wifi_manager.h/.cpp   # WiFi Manager (captive portal)
│   ├── render.h/.cpp         # Dashboard renderer (partial update)
│   ├── race_logic.h/.cpp     # Sorting and gap calculation
│   └── ticker.h/.cpp         # Race commentary ticker
├── test/
│   └── test_native/          # Unit tests (GoogleTest + RapidCheck)
├── docs/
│   ├── WIRING_GUIDE.md       # Full wiring diagrams
│   ├── NETWORK_ARCHITECTURE.md # WiFi/webhook design decisions
│   ├── HARDWARE_INVENTORY.md # BOM and purchase links
│   ├── ENCLOSURE_SPEC.md     # 3D printed case specs
│   └── PENDING_ISSUES.md     # Known issues tracker
├── platformio.ini            # Build configuration (3 environments)
└── README.md
```

## Build Environments

| Environment | Command | Description |
|-------------|---------|-------------|
| `esp32s3` | `pio run -e esp32s3` | Simulation mode (no WiFi) |
| `esp32s3_fpvgate` | `pio run -e esp32s3_fpvgate` | FPVGate integration (WiFi + webhooks) |
| `native` | `pio test -e native` | Host-based unit tests |

---

## FAQ

### Why does FPVGate need an external WiFi network? Can't the display connect directly to FPVGate's AP?

FPVGate's webhook code checks `WiFi.status() == WL_CONNECTED` before sending. This is only `true` when the ESP32 is connected **as a client (STA)** to another network. When FPVGate **is** the AP, that condition is always `false` and webhooks never fire.

The solution: all devices (timers + display) connect to the same external WiFi (your home router or a phone hotspot). FPVGate was designed for multi-node setups where a central router connects everything.

> Full explanation: [docs/NETWORK_ARCHITECTURE.md](docs/NETWORK_ARCHITECTURE.md)

### Do I need internet access?

No. The WiFi network only provides local connectivity between devices. No internet is required. A phone hotspot with mobile data disabled works fine.

### How do I change WiFi networks (e.g., moving from home to a race field)?

The display uses WiFi Manager with a captive portal. To switch networks:
1. The display will fail to connect to the old network
2. After timeout, it opens the "RaceGate_Display" AP again
3. Connect from your phone and select the new network

Alternatively, you can trigger a WiFi reset by reflashing the firmware.

### Can I use this display without FPVGate?

Yes. Flash with `pio run -e esp32s3 -t upload` for simulation mode. It runs a realistic race demo with 6 pilots — useful for development, showcasing, or as a base for integrating with other timing systems.

### How does multi-pilot work?

Each pilot needs their own FPVGate timer node (XIAO + RX5808, ~$12 each) tuned to a different video channel. All timers connect to the same WiFi and send webhooks to the display. The display identifies each pilot by the source IP of the webhook.

### What happens with only one timer?

The display detects it's a single-pilot session and switches to lap history mode — showing each lap as a row with time and gap vs personal best. When multiple timers are present, it automatically switches to the multi-pilot leaderboard.

---

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for release history.

## Contributing

This project is built with [Kiro](https://kiro.dev) AI-powered IDE. Contributions welcome — open an issue or PR.

## License

MIT

## Credits

Created by **Verma FPV** (Jose Vega)

- GitHub: [@jsvegam](https://github.com/jsvegam)
- Instagram: [@VemarFPV](https://instagram.com/VemarFPV)
- Email: jsvegam@gmail.com

Built for the FPV racing community. Questions, ideas, or want to collaborate? Open an issue or reach out directly.
