# RaceGate Hub

Display standalone de carrera para sistemas de laptimer FPV. Muestra un dashboard en tiempo real con posiciones, tiempos de vuelta, mejores tiempos, número de vueltas y gaps entre pilotos.

Construido con un **Seeed Studio XIAO ESP32-S3** y una pantalla **ILI9488 3.5" TFT (480x320)** conectada por SPI.

## Estado actual

**Fase 1 — Standalone con datos simulados.** El sistema funciona de forma autónoma con un motor de simulación que genera datos de carrera realistas. La arquitectura está preparada para reemplazar la simulación por datos reales de FPVGate vía WiFi.

## Características

- Dashboard tipo tabla con 5-8 pilotos simultáneos
- Columnas: Posición, Nombre, Última Vuelta, Mejor Vuelta, Vueltas, Gap
- Tema oscuro con colores de destaque (verde = líder, rojo = peor tiempo)
- Ordenamiento dinámico de posiciones
- Renderizado eficiente con actualizaciones parciales (sin parpadeo)
- Tasa de refresco: 5-10 FPS
- Interfaz Data_Provider abstracta para futura integración WiFi

## Hardware

| Componente | Modelo |
|------------|--------|
| MCU | Seeed Studio XIAO ESP32-S3 |
| Pantalla | ILI9488 3.5" TFT SPI (480x320) |
| Alimentación | 5V vía USB (desarrollo) o boost converter (standalone) |

### Cableado SPI

| Señal | GPIO ESP32-S3 | Pin XIAO | Pin ILI9488 |
|-------|---------------|----------|-------------|
| MOSI | GPIO10 | D10 | SDI (MOSI) |
| MISO | GPIO9 | D9 | SDO (MISO) |
| SCK | GPIO8 | D8 | SCK |
| CS | GPIO2 | D0 | CS |
| DC | GPIO4 | D2 | DC/RS |
| RST | GPIO5 | D3 | RESET |
| BL | GPIO3 | D1 | LED (Backlight) |

Además conectar:
- **5V del XIAO** → **VCC del TFT**
- **GND del XIAO** → **GND del TFT**

### Alimentación durante desarrollo

```
PC ──USB-C──► XIAO ESP32-S3 ──pin 5V──► ILI9488 VCC
                              ──GND────► ILI9488 GND
```

El cable USB-C alimenta el XIAO (~100 mA) y el XIAO alimenta la pantalla (~150-200 mA) por el pin 5V. Consumo total ~300 mA, dentro del límite de un puerto USB estándar. No se necesita fuente externa.

### Alimentación standalone (sin PC)

```
Batería LiPo ──► Boost Converter (5V, ≥500mA) ──► pin 5V del XIAO
                                                 ──► GND del XIAO
```

No conectar USB y fuente externa simultáneamente.

## Requisitos de software

- [PlatformIO](https://platformio.org/) (CLI o extensión VS Code / Kiro)
- Framework: Arduino

## Compilar y flashear

```bash
# Compilar
pio run

# Flashear al XIAO ESP32-S3
pio run --target upload

# Monitor serial (opcional, para debug)
pio device monitor --baud 115200
```

## Estructura del proyecto

```
├── src/
│   ├── main.cpp              # Loop principal (orquestador)
│   ├── display.h / .cpp      # Inicialización del hardware TFT
│   ├── data_provider.h       # Interfaz abstracta de datos
│   ├── simulation.h / .cpp   # Motor de simulación de carrera
│   └── render.h / .cpp       # Renderizado del dashboard
├── test/
│   └── test_native/          # Tests unitarios y PBT (host)
├── platformio.ini            # Configuración PlatformIO
└── README.md
```

## Arquitectura

```
┌──────────┐     ┌───────────────┐     ┌──────────────┐
│Main Loop │────►│ Data_Provider │◄────│  Simulation  │
│          │     │  (interfaz)   │     │   Engine     │
│          │     └───────────────┘     └──────────────┘
│          │            ▲
│          │            │ (futuro)
│          │     ┌──────┴────────┐
│          │     │ WiFi/FPVGate  │
│          │     │   Client      │
│          │     └───────────────┘
│          │
│          │────►┌───────────────┐     ┌──────────────┐
│          │     │ Render_Engine │────►│  TFT_eSPI    │
└──────────┘     └───────────────┘     │  ILI9488     │
                                       └──────────────┘
```

La interfaz `DataProvider` desacopla la fuente de datos del renderizado. Para integrar FPVGate, solo hay que crear una nueva clase que implemente `DataProvider` (por ejemplo vía WebSocket o HTTP polling) sin modificar el código de renderizado.

## Roadmap

- [x] Fase 1: Display standalone con simulación
- [ ] Fase 2: Integración WiFi con [FPVGate](https://github.com/LouisHitchcock/FPVGate) (Webhooks / HTTP polling)
- [ ] Fase 3: Configuración por interfaz (número de pilotos, colores, etc.)

## Integración con FPVGate

Este display está diseñado para integrarse con [FPVGate](https://github.com/LouisHitchcock/FPVGate), un laptimer FPV basado en RSSI que usa el mismo XIAO ESP32-S3. La integración se hará conectando el display como cliente WiFi al AP de FPVGate y recibiendo datos de carrera vía HTTP webhooks.

La interfaz `DataProvider` permite reemplazar la simulación por datos reales sin modificar el código de renderizado. Solo se necesita implementar una clase `FPVGateProvider` que se conecte al WiFi y parsee los datos de carrera.

Ver detalles técnicos en el [documento de diseño](.kiro/specs/fpv-laptimer-display/design.md#plan-de-integración-con-fpvgate).

## Preguntas Frecuentes

### ¿Necesito conectar la pantalla para flashear el firmware?

No. Puedes conectar solo el XIAO ESP32-S3 por USB-C a tu PC y flashear. El firmware arranca, intenta inicializar la pantalla (no falla si no está conectada) y puedes verificar por monitor serial que funciona. Después conectas la pantalla y reconectas el USB.

Se recomienda hacerlo en dos pasos:
1. Solo XIAO por USB → flashear y verificar por serial
2. XIAO + pantalla cableada → reconectar USB y ver el dashboard

### ¿Cómo se alimenta la pantalla durante desarrollo?

El cable USB-C de tu PC alimenta el XIAO con 5V. El pin 5V del XIAO saca esos 5V directamente (passthrough del USB) y alimenta la pantalla. Consumo total ~300 mA, dentro del límite de un puerto USB estándar. No necesitas fuente externa mientras desarrollas.

### ¿Qué es el SimulationEngine y para qué sirve?

Es un "modo demo" que simula lo que haría FPVGate. Genera 6 pilotos ficticios con nombres predefinidos (RAZOR, VIPER, GHOST, etc.), cada uno con velocidad base diferente. Cada 2-4 segundos un piloto completa una vuelta con un tiempo aleatorio entre 10 y 20 segundos. Las posiciones se recalculan dinámicamente. Esto permite probar y demostrar el display sin necesitar el hardware de laptimer.

### ¿Este display necesita el firmware de FPVGate?

No. Son dos dispositivos completamente independientes, cada uno con su propio XIAO ESP32-S3 y su propio firmware:

- **XIAO #1 (FPVGate)**: tiene el sensor RX5808, detecta drones por RSSI, mide tiempos. Se flashea con el firmware del [repo de FPVGate](https://github.com/LouisHitchcock/FPVGate).
- **XIAO #2 (este proyecto)**: tiene la pantalla ILI9488, muestra el dashboard. Se flashea con el firmware de este repo.

En la Fase 2, el display se conecta al WiFi que crea FPVGate y consume los datos de carrera. No necesitas tocar el código de FPVGate.

### ¿Cómo funciona FPVGate en modo multipiloto?

FPVGate usa un solo receptor RX5808 que escanea múltiples frecuencias. Cada piloto vuela en un canal diferente de RaceBand (R1=5658MHz, R2=5695MHz, etc., hasta 8 canales). Los pilotos se configuran desde la interfaz web de FPVGate donde asignas nombre, canal y umbral RSSI. Cuando un dron pasa por la gate, FPVGate detecta el pico RSSI en la frecuencia de ese piloto y registra la vuelta.

### ¿Cómo se registran los nombres de pilotos?

En la Fase 1 (actual), los nombres son predefinidos en el SimulationEngine. En la Fase 2 (integración real), los nombres vendrán de la configuración que hagas en la interfaz web de FPVGate. Tu display solo los recibe y muestra — no necesita saber nada de frecuencias ni configuración de pilotos.

### ¿Puedo usar este display sin FPVGate?

Sí. En la Fase 1 funciona completamente standalone con datos simulados. Es útil como demo, para desarrollo, o como base para integrar con cualquier otro sistema de laptimer que exponga datos por WiFi.

## Licencia

Por definir.
