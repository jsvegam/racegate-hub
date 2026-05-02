# Hardware Inventory — RaceGate Hub

## Estado actual del proyecto

### ✅ En uso (Fase 1 — Display standalone)

| Componente | Modelo | Estado |
|------------|--------|--------|
| MCU | Seeed XIAO ESP32-S3 | ✅ Funcionando |
| Pantalla | TFT 3.5" ILI9488 SPI (480x320) | ✅ Funcionando |
| Alimentación | Power bank USB-C | ✅ Suficiente para Fase 1 y 2 |

### 📦 En camino

| Componente | Modelo | Para qué |
|------------|--------|----------|
| Módulo RF | RX5808 (RSSI analógico) | Laptimer FPVGate (Dispositivo 2) |

### 📦 En inventario (para futuro)

| Componente | Modelo | Notas |
|------------|--------|-------|
| Batería | LiPo 3.7V 2000mAh (103450) | Para case todo-en-uno futuro |
| Cargador | TP4056 (sin protección ⚠️) | Necesita BMS 1S o TP4056 con protección |
| Boost converter | MT3608 (step-up a 5V) | Para alimentar desde LiPo |
| Conectores | JST PH 2.0 | Conexiones internas |
| USB-C breakout | 4 pin | Puerto de carga externo |
| Switches | SS12D00 deslizantes | On/off |
| Separadores | Brass standoffs | Montaje interno |
| Cables | Dupont hembra-hembra | Prototipado |

### ❌ No compatible

| Componente | Modelo | Problema |
|------------|--------|----------|
| Módulo RF | RX5808 SPI (TX5813 variant) | No entrega RSSI analógico usable para laptimer |

---

## Arquitectura de dispositivos

```
Dispositivo 1: FPVGate (laptimer)
┌─────────────────────────────┐
│ XIAO ESP32-S3               │
│ + RX5808 (RSSI analógico)   │
│ + Antena SMA 5.8GHz         │
│ Alimentación: Power bank    │
│ WiFi: Access Point           │
│ Firmware: FPVGate            │
└─────────────────────────────┘
              │ WiFi
              ▼
Dispositivo 2: RaceGate Hub (display)
┌─────────────────────────────┐
│ XIAO ESP32-S3               │
│ + ILI9488 3.5" TFT          │
│ Alimentación: Power bank    │
│ WiFi: Cliente                │
│ Firmware: Este repo          │
└─────────────────────────────┘
```

---

## Fases del proyecto

### Fase 1 — Display standalone (COMPLETADA ✅)
- Dashboard con simulación de carrera
- 6 pilotos, posiciones, gaps, tiempos
- Ticker con comentarios y efecto partículas
- Alimentación: USB-C (PC o power bank)

### Fase 2 — Integración WiFi con FPVGate
- Display se conecta al WiFi AP de FPVGate
- Recibe datos de carrera reales vía HTTP webhooks
- Requiere: RX5808 + segundo XIAO con firmware FPVGate

### Fase 3 — Case impreso 3D
- Diseño que aloje: XIAO + pantalla + power bank
- Pantalla frontal flush
- Puerto USB-C accesible para carga
- Espacio interno preparado para futuro upgrade a batería LiPo integrada

### Fase 4 — Batería integrada (futuro)
- Reemplazar power bank por LiPo interna
- TP4056 con protección + MT3608 boost a 5V
- Switch on/off
- Indicador de carga

### Fase 5 — Audio (futuro)
- DFPlayer Mini + speaker 3W 4Ω + microSD
- Anuncios de vueltas, posiciones, mejores tiempos

---

## Lo que falta comprar (para fases futuras)

| Prioridad | Componente | Para qué |
|-----------|------------|----------|
| 🔴 Alta | Antena SMA 5.8GHz + pigtail | FPVGate (Fase 2) |
| 🟡 Media | TP4056 CON protección (o BMS 1S) | Batería segura (Fase 4) |
| 🟡 Media | DFPlayer Mini + speaker + microSD | Audio (Fase 5) |
| 🟢 Baja | AMS1117 3.3V + capacitor 470-1000uF | Regulación limpia |
| 🟢 Baja | Tornillos M2/M2.5 + heat-set inserts | Case (Fase 3) |
