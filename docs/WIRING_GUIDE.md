# Guía de Cableado — RaceGate Hub

## Diagrama General del Sistema

```
    Dispositivo 1: TIMER                      Dispositivo 2: DISPLAY
  ┌───────────────────────┐                 ┌───────────────────────┐
  │  XIAO ESP32-S3        │                 │  XIAO ESP32-S3        │
  │  + RX5808             │  WiFi 802.11n   │  + ILI9488 TFT 3.5"  │
  │  + Antena 5.8GHz      │ ═══════════════ │  480x320 px           │
  │                       │  AP ──► Cliente  │                       │
  │  Alimentación: USB-C  │                 │  Alimentación: USB-C  │
  └───────────────────────┘                 └───────────────────────┘
```

---

## Dispositivo 2 — DISPLAY (ya armado ✅)

### Pinout XIAO ESP32-S3 → ILI9488 TFT

```
                 Seeed Studio XIAO ESP32-S3
                 ┌─────────────────────┐
                 │       USB-C          │
                 │      ┌─────┐         │
                 │      └─────┘         │
                 │                      │
          5V  ●──┤ 5V            D10 ├──● GPIO10 ──► MOSI (TFT)
         GND  ●──┤ GND            D9 ├──● GPIO9  ◄── MISO (TFT)
   CS (TFT)◄──●──┤ D0 (GPIO2)     D8 ├──● GPIO8  ──► SCK  (TFT)
   BL (TFT)◄──●──┤ D1 (GPIO3)     D7 ├──● (libre)
   DC (TFT)◄──●──┤ D2 (GPIO4)     D6 ├──● (libre)
  RST (TFT)◄──●──┤ D3 (GPIO5)     D5 ├──● (libre)
       (libre)●──┤ D4 (GPIO6)         │
                 │                      │
                 │  LED: GPIO21         │
                 └──────────────────────┘
```

### Tabla de conexiones con colores de cable

```
═══════════════════════════════════════════════════════════════════════
 CABLEADO DISPLAY: XIAO ESP32-S3 → ILI9488 TFT 3.5"
═══════════════════════════════════════════════════════════════════════

 Color Cable     Pin XIAO          Pin ILI9488      Función
───────────────────────────────────────────────────────────────────────
 🔴 ROJO         5V          ───►  VCC              Alimentación 5V
 ⚫ NEGRO        GND         ───►  GND              Tierra
 🟣 MORADO       D10 (GPIO10)───►  SDI (MOSI)       Datos SPI salida
 ⚪ BLANCO       D9  (GPIO9) ◄───  SDO (MISO)       Datos SPI entrada
 🔵 AZUL         D8  (GPIO8) ───►  SCK              Reloj SPI
 🟡 AMARILLO     D0  (GPIO2) ───►  CS               Chip Select
 🟢 VERDE        D2  (GPIO4) ───►  DC/RS            Data/Command
 🟠 NARANJA      D3  (GPIO5) ───►  RESET            Reset pantalla
 🟤 MARRÓN       D1  (GPIO3) ───►  LED              Backlight
───────────────────────────────────────────────────────────────────────
 Total: 9 cables
═══════════════════════════════════════════════════════════════════════
```

### Diagrama del módulo TFT ILI9488

```
                    ILI9488 TFT 3.5" (480x320)
              ┌─────────────────────────────────┐
              │                                 │
              │         ┌───────────┐           │
              │         │  PANTALLA │           │
              │         │  480x320  │           │
              │         │           │           │
              │         └───────────┘           │
              │                                 │
              │  Pines (borde inferior):        │
              │                                 │
 🔴 ROJO ────┤  VCC                            │
 ⚫ NEGRO ───┤  GND                            │
 🟡 AMARILLO─┤  CS                             │
 🟠 NARANJA──┤  RESET                          │
 🟢 VERDE ───┤  DC/RS                          │
 🟣 MORADO ──┤  SDI (MOSI)                     │
 🔵 AZUL ────┤  SCK                            │
 🟤 MARRÓN ──┤  LED (Backlight)                │
 ⚪ BLANCO ──┤  SDO (MISO)                     │
              │                                 │
              └─────────────────────────────────┘
```

---

## Dispositivo 1 — TIMER (RX5808 + FPVGate)

> **Firmware**: [FPVGate](https://github.com/LouisHitchcock/FPVGate) v1.7.x
> **Target PlatformIO**: `SeeedXIAOESP32S3`
> **Pinout según**: docs/wiring/Seeed-XIAO-ESP32S3.md del repo FPVGate

### Pinout XIAO ESP32-S3 → RX5808 (pinout oficial FPVGate)

```
                 Seeed Studio XIAO ESP32-S3
                 ┌─────────────────────┐
                 │       USB-C          │
                 │      ┌─────┐         │
                 │      └─────┘         │
                 │                      │
          5V  ●──┤ 5V            D10 ├──● (libre)
         GND  ●──┤ GND            D9 ├──● (libre)
       (libre)●──┤ D0 (GPIO1)     D8 ├──● (libre)
       (libre)●──┤ D1 (GPIO2)     D7 ├──● GPIO44 (libre)
  RSSI ◄──────●──┤ D2 (GPIO3)     D6 ├──● GPIO43 (libre)
  CH3 (CLK)◄──●──┤ D3 (GPIO4)     D5 ├──● GPIO6 ──► CH2 (SELECT)
  CH1 (DATA)◄─●──┤ D4 (GPIO5)         │
                 │                      │
                 │  LED: GPIO21         │
                 └──────────────────────┘
```

### Tabla de conexiones con colores de cable

```
═══════════════════════════════════════════════════════════════════════
 CABLEADO TIMER: XIAO ESP32-S3 → RX5808 (pinout FPVGate)
═══════════════════════════════════════════════════════════════════════

 Color Cable     Pin XIAO          Pin RX5808       Función
───────────────────────────────────────────────────────────────────────
 🔴 ROJO         5V          ───►  +5V              Alimentación
 ⚫ NEGRO        GND         ───►  GND              Tierra
 🟡 AMARILLO     D2 (GPIO3)  ◄───  RSSI             Señal analógica ADC
 🟢 VERDE        D4 (GPIO5)  ───►  CH1              SPI Data
 🔵 AZUL         D5 (GPIO6)  ───►  CH2              SPI Select
 🟠 NARANJA      D3 (GPIO4)  ───►  CH3              SPI Clock
───────────────────────────────────────────────────────────────────────
 Total: 6 cables (Dupont cortados, soldar extremo al RX5808)
═══════════════════════════════════════════════════════════════════════
```

### Diagrama del módulo RX5808

```
                         RX5808 Module
              ┌──────────────────────────────────┐
              │          (vista trasera)          │
              │                                  │
              │  ┌────┐         ┌──────────┐     │
              │  │ IC │         │ Cristal  │     │
              │  └────┘         └──────────┘     │
              │                                  │
  Lado izq:   │                    Lado derecho: │
              │                                  │
 ⚫ NEGRO ────┤ GND                  CH1 ├────── 🟢 VERDE
 (antena) ────┤ ANT                  CH2 ├────── 🔵 AZUL
 ⚫ (no usar)─┤ GND                  CH3 ├────── 🟠 NARANJA
              │                      GND ├────── (no usar)
              │                      +5V ├────── 🔴 ROJO
              │                     RSSI ├────── 🟡 AMARILLO
              │                    A6.5M ├────── ✕ NO CONECTAR
              │                    Video ├────── ✕ NO CONECTAR
              │                      GND ├────── (no usar)
              │                                  │
              └──────────────────────────────────┘

 NOTAS:
 • ANT → Antena 5.8GHz (cloverleaf, dipolo, o patch)
 • Video → No se usa para laptimer
 • A6.5M → Referencia cristal interno, NO TOCAR
 • Solo necesitás conectar 1 GND (cualquiera de los 3)
```

### Cómo flashear FPVGate

```
═══════════════════════════════════════════════════════════════════════
 OPCIÓN A — Web Flasher (recomendado, sin instalar nada)
═══════════════════════════════════════════════════════════════════════
 1. Abrí Chrome/Edge/Opera
 2. Andá a: https://fpvgate.xyz/flasher.html
 3. Conectá el XIAO ESP32-S3 por USB-C (sin nada más conectado)
 4. Seleccioná "Seeed Studio XIAO ESP32S3 (8MB)"
 5. Click "Connect" → elegí el puerto serial
 6. Click "Flash" → esperá ~2-3 minutos
═══════════════════════════════════════════════════════════════════════

═══════════════════════════════════════════════════════════════════════
 OPCIÓN B — PlatformIO (si querés compilar desde fuente)
═══════════════════════════════════════════════════════════════════════
 git clone https://github.com/LouisHitchcock/FPVGate.git
 cd FPVGate
 pio run -e SeeedXIAOESP32S3 -t upload
 pio run -e SeeedXIAOESP32S3 -t uploadfs
═══════════════════════════════════════════════════════════════════════
```

### Verificación post-flash (antes de soldar)

```
 1. Flasheá el XIAO (sin RX5808 conectado)
 2. El XIAO levanta WiFi AP: "FPVGate_XXXX" (password: fpvgate1)
 3. Conectate desde celular/PC al WiFi
 4. Abrí http://192.168.4.1
 5. Si ves la interfaz web de FPVGate → firmware OK ✅
 6. Ahora sí desconectá, soldá los Dupont al RX5808, y conectá
```

---

## Resumen de compras de cables

| Dispositivo | Cables Dupont necesarios | Tipo |
|-------------|--------------------------|------|
| Display (TFT) | 9 cables hembra-hembra | Enchufar ambos extremos |
| Timer (RX5808) | 6 cables hembra-hembra | Cortar un extremo, soldar al RX5808 |

### Código de colores completo

```
═══════════════════════════════════════════════════════════════
 LEYENDA DE COLORES (ambos dispositivos)
═══════════════════════════════════════════════════════════════
 🔴 ROJO      = Alimentación (+5V)
 ⚫ NEGRO     = Tierra (GND)
 🟡 AMARILLO  = CS (display) / RSSI (timer)
 🟢 VERDE     = DC (display) / CH1 Data (timer)
 🔵 AZUL      = SCK (display) / CH2 Select (timer)
 🟠 NARANJA   = RESET (display) / CH3 Clock (timer)
 🟣 MORADO    = MOSI (solo display)
 ⚪ BLANCO    = MISO (solo display)
 🟤 MARRÓN    = Backlight (solo display)
═══════════════════════════════════════════════════════════════
```

---

## Alimentación

### Durante desarrollo (ambos dispositivos)

```
PC ──USB-C──► XIAO (Timer)   → alimenta RX5808 por pin 5V
PC ──USB-C──► XIAO (Display) → alimenta TFT por pin 5V
```

### En campo (standalone)

```
Power bank USB ──► XIAO (Timer)   → alimenta RX5808
Power bank USB ──► XIAO (Display) → alimenta TFT
```

Un solo power bank con 2 puertos USB sirve para ambos dispositivos.
Consumo total estimado: ~300mA (display) + ~200mA (timer) = ~500mA.
