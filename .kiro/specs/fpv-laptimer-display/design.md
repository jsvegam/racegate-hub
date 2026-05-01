# Documento de Diseño — FPV Laptimer Display

## Resumen

Este documento describe el diseño técnico del sistema FPV Laptimer Display, un dashboard de carrera standalone basado en un Seeed Studio XIAO ESP32-S3 conectado a una pantalla TFT ILI9488 de 3.5" (480x320) vía SPI. El sistema muestra en tiempo real posiciones, tiempos de vuelta, mejores tiempos, número de vueltas y gaps entre pilotos. En esta fase inicial, un motor de simulación genera datos de carrera realistas; la arquitectura permite reemplazar esta fuente por datos reales de FPVGate sin modificar el código de renderizado.

El diseño prioriza tres aspectos clave:
1. **Renderizado eficiente**: Actualizaciones parciales de pantalla para evitar parpadeo y mantener 5-10 FPS.
2. **Modularidad**: Separación clara entre hardware, datos, simulación y renderizado.
3. **Extensibilidad**: Abstracción Data_Provider que desacopla la fuente de datos del renderizado.

## Arquitectura

### Diagrama de Arquitectura General

```
┌─────────────────────────────────────────────────────────────────┐
│                        SOFTWARE LAYER                           │
│                                                                 │
│  ┌────────────┐    inicializa    ┌─────────────────┐            │
│  │ Main Loop  │────────────────►│ Display Module   │            │
│  │            │                  └────────┬────────┘            │
│  │            │    inicializa             │ inicializa           │
│  │            │──────────┐               │ hardware             │
│  │            │          ▼               │                      │
│  │            │   ┌──────────────┐       │                      │
│  │            │   │  Simulation  │       │                      │
│  │            │   │   Engine     │       │                      │
│  │            │   └──────┬───────┘       │                      │
│  │            │          │ implementa    │                      │
│  │            │          ▼               │                      │
│  │            │  ┌───────────────────┐   │                      │
│  │            │  │  Data_Provider    │   │                      │
│  │  consulta  │  │   (interfaz)     │   │                      │
│  │  datos     │  ├───────────────────┤   │                      │
│  │───────────►│  │ · SimulationEngine│   │                      │
│  │            │  │ · WiFi Client (*)│   │                      │
│  │            │  └───────────────────┘   │                      │
│  │            │                          │                      │
│  │  envía     │  ┌───────────────┐       │                      │
│  │  datos     │  │ Render_Engine │       │                      │
│  │───────────►│  │               │───────┘                      │
│  └────────────┘  └───────┬───────┘  dibuja vía TFT_eSPI        │
│                          │                                      │
└──────────────────────────┼──────────────────────────────────────┘
                           │
┌──────────────────────────┼──────────────────────────────────────┐
│                     HARDWARE LAYER                              │
│                          ▼                                      │
│              ┌──────────────────┐                               │
│              │  XIAO ESP32-S3   │                               │
│              └────────┬─────────┘                               │
│                       │ SPI Bus                                 │
│                       ▼                                         │
│              ┌──────────────────┐                               │
│              │ ILI9488 3.5" TFT │                               │
│              │   480 x 320      │                               │
│              └──────────────────┘                               │
└─────────────────────────────────────────────────────────────────┘

(*) WiFi/FPVGate Client = implementación futura
```

### Flujo de Ejecución

```
Main Loop          Display Module     Simulation       Render Engine      TFT/ILI9488
    │                    │                │                 │                  │
    │  init_display()    │                │                 │                  │
    │───────────────────►│                │                 │                  │
    │                    │  Configurar SPI + ILI9488        │                  │
    │                    │────────────────────────────────────────────────────►│
    │                    │  Mostrar splash screen (2s)      │                  │
    │                    │────────────────────────────────────────────────────►│
    │                    │                │                 │                  │
    │  init(num_pilots)  │                │                 │                  │
    │────────────────────────────────────►│                 │                  │
    │                    │                │                 │                  │
    │ ╔══════════════════════════════════════════════════════════════════╗     │
    │ ║  LOOP: Cada ciclo (~100-200ms)                                 ║     │
    │ ║                                                                ║     │
    │ ║  update()                         │                 │          ║     │
    │ ║─────────────────────────────────►│                 │          ║     │
    │ ║                                   │                 │          ║     │
    │ ║  has_new_data()                   │                 │          ║     │
    │ ║─────────────────────────────────►│                 │          ║     │
    │ ║                                   │                 │          ║     │
    │ ║  [Si hay datos nuevos]            │                 │          ║     │
    │ ║  get_pilots()                     │                 │          ║     │
    │ ║─────────────────────────────────►│                 │          ║     │
    │ ║                                   │                 │          ║     │
    │ ║  render_dashboard(pilots)         │                 │          ║     │
    │ ║────────────────────────────────────────────────────►│          ║     │
    │ ║                                   │    Comparar con frame ant. │     │
    │ ║                                   │                 │──┐       ║     │
    │ ║                                   │                 │◄─┘       ║     │
    │ ║                                   │    Redibujar celdas cambiadas    │
    │ ║                                   │                 │─────────────►│ ║
    │ ║                                                                ║     │
    │ ╚══════════════════════════════════════════════════════════════════╝     │
```

### Diagrama de Componentes

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           main.cpp                                      │
│                        (Orquestador)                                    │
│   setup() ──► init_display() ──► init_simulation() ──► init_render()   │
│   loop()  ──► update() ──► has_new_data() ──► render_dashboard()       │
└──────┬──────────────┬──────────────────┬────────────────┬───────────────┘
       │              │                  │                │
       ▼              ▼                  ▼                ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌──────────────────┐
│ display.h/cpp│ │data_provider │ │simulation.h/ │ │  render.h/cpp    │
│              │ │     .h       │ │    cpp        │ │                  │
│ Funciones:   │ │ Interfaz:    │ │ Clase:        │ │ Funciones:       │
│ ·init_display│ │ ·update()    │ │ Simulation    │ │ ·init_render()   │
│ ·show_splash │ │ ·has_new_    │ │  Engine       │ │ ·draw_header()   │
│ ·signal_error│ │   data()     │ │              │ │ ·render_         │
│              │ │ ·get_pilots()│ │ Hereda de:    │ │   dashboard()    │
│ Dependencias:│ │              │ │ DataProvider  │ │                  │
│ ·TFT_eSPI   │ │ Structs:     │ │              │ │ Structs:         │
│              │ │ ·PilotEntry  │ │ Estado:       │ │ ·CellCache       │
│              │ │              │ │ ·PilotState[] │ │ ·RenderState     │
│              │ │ Constantes:  │ │ ·num_pilots_  │ │                  │
│              │ │ ·MAX_PILOTS  │ │ ·data_changed_│ │ Constantes:      │
│              │ │ ·NAME_MAX_LEN│ │              │ │ ·SCREEN_W/H      │
└──────┬───────┘ └──────┬───────┘ └──────┬───────┘ │ ·HEADER_H/ROW_H  │
       │                │                │         │ ·NUM_COLS         │
       │                │    implementa  │         └────────┬─────────┘
       │                │◄───────────────┘                  │
       │                │                                   │
       │                │◄ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
       │                │         consume (lee pilotos)
       │                │
       ▼                │
┌──────────────┐        │         ┌─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐
│  TFT_eSPI    │        │           WiFi/FPVGate Client
│  (librería)  │        │◄ ─ ─ ─ ─│  (futuro)             │
│              │        │           Implementará
│ ·tft.init()  │        │         │ DataProvider            │
│ ·tft.fillRect│                   ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─
│ ·tft.print() │
│ ·tft.setTextColor()│
└──────┬───────┘
       │ SPI Bus
       ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                    HARDWARE — Seeed Studio XIAO ESP32-S3                │
│                                                                         │
│  ┌───────────────────────────────┐    ┌──────────────────────────────┐  │
│  │        MCU ESP32-S3           │    │     ILI9488 3.5" TFT        │  │
│  │                               │    │       480 x 320 px          │  │
│  │  CPU: Xtensa LX7 dual-core   │    │                              │  │
│  │  Flash: 8MB                   │    │  Interfaz: SPI (RGB666)     │  │
│  │  PSRAM: 8MB                   │    │  Backlight: controlado      │  │
│  │  WiFi: 802.11 b/g/n (futuro) │    │    por GPIO                 │  │
│  │  GPIO: 11 pines disponibles   │    │                              │  │
│  │                               │    │                              │  │
│  │  Pines SPI asignados:        │    │  Pines TFT:                 │  │
│  │   MOSI ── GPIO10 (D10) ──────┼────┼──► SDI (MOSI)               │  │
│  │   MISO ── GPIO9  (D9)  ◄─────┼────┼─── SDO (MISO)               │  │
│  │   SCK  ── GPIO8  (D8)  ──────┼────┼──► SCK                      │  │
│  │   CS   ── GPIO2  (D0)  ──────┼────┼──► CS                       │  │
│  │   DC   ── GPIO4  (D2)  ──────┼────┼──► DC/RS                    │  │
│  │   RST  ── GPIO5  (D3)  ──────┼────┼──► RESET                    │  │
│  │   BL   ── GPIO3  (D1)  ──────┼────┼──► LED (Backlight)          │  │
│  │                               │    │                              │  │
│  └───────────────────────────────┘    └──────────────────────────────┘  │
│                                                                         │
│  Alimentación: 5V (boost converter) ──► VIN                            │
│  LED integrado: GPIO21 (señalización de errores)                       │
└─────────────────────────────────────────────────────────────────────────┘

Leyenda:
  ──────►  Dependencia directa / llamada
  ─ ─ ─ ►  Dependencia a través de interfaz
  ◄ ─ ─ ─  Implementación futura
```

### Decisiones de Diseño

| Decisión | Elección | Justificación |
|----------|----------|---------------|
| Biblioteca gráfica | TFT_eSPI | Optimizada para ESP32, soporte nativo ILI9488, SPI DMA |
| Estrategia de renderizado | Partial Update por celda | Evita parpadeo, reduce tráfico SPI, mantiene 5-10 FPS |
| Abstracción de datos | Interfaz virtual C++ (Data_Provider) | Permite intercambiar simulación por WiFi sin tocar renderizado |
| Formato de tiempos | Enteros en milisegundos | Evita aritmética de punto flotante, más preciso y rápido |
| Ordenamiento | En Data_Provider, no en Render_Engine | Separa lógica de negocio de presentación |
| Build system | PlatformIO | Estándar para ESP32, gestión de dependencias integrada |

## Componentes e Interfaces

### Módulo: Display (`display.h` / `display.cpp`)

Responsable de la inicialización del hardware TFT y la pantalla de inicio.

```cpp
// display.h
#pragma once

#include <TFT_eSPI.h>

namespace display {
    // Inicializa el TFT_Display vía SPI y muestra splash screen.
    // Retorna true si la inicialización fue exitosa.
    bool init_display(TFT_eSPI& tft);

    // Muestra la pantalla de inicio con el nombre del sistema.
    void show_splash_screen(TFT_eSPI& tft);

    // Señaliza error mediante LED integrado (parpadeo 5Hz).
    void signal_error();
}
```

### Interfaz: Data_Provider (`data_provider.h`)

Interfaz abstracta que desacopla la fuente de datos del renderizado.

```cpp
// data_provider.h
#pragma once

#include <cstdint>

static const uint8_t MAX_PILOTS = 8;
static const uint8_t NAME_MAX_LEN = 12;

struct PilotEntry {
    uint8_t  position;                // Posición actual (1-8)
    char     name[NAME_MAX_LEN + 1];  // Nombre del piloto (null-terminated)
    uint32_t last_lap_ms;             // Último tiempo de vuelta en ms
    uint32_t best_lap_ms;             // Mejor tiempo de vuelta en ms
    uint16_t lap_count;               // Número total de vueltas completadas
    int32_t  gap_ms;                  // Gap respecto al líder en ms (-1 = líder)
    uint32_t total_time_ms;           // Tiempo acumulado total en ms
};

class DataProvider {
public:
    virtual ~DataProvider() = default;

    // Actualiza el estado interno (avanza simulación, recibe datos WiFi, etc.)
    virtual void update() = 0;

    // Retorna true si hay datos nuevos desde la última llamada a get_pilots().
    virtual bool has_new_data() const = 0;

    // Llena el array de pilotos ordenados por posición.
    // Retorna el número de pilotos activos (5-8).
    virtual uint8_t get_pilots(PilotEntry pilots[], uint8_t max_count) = 0;
};
```

### Módulo: Simulation Engine (`simulation.h` / `simulation.cpp`)

Implementa `DataProvider` generando datos de carrera simulados.

```cpp
// simulation.h
#pragma once

#include "data_provider.h"

class SimulationEngine : public DataProvider {
public:
    // Inicializa la simulación con num_pilots pilotos (5-8).
    void init(uint8_t num_pilots);

    void update() override;
    bool has_new_data() const override;
    uint8_t get_pilots(PilotEntry pilots[], uint8_t max_count) override;

private:
    struct PilotState {
        char     name[NAME_MAX_LEN + 1];
        uint32_t base_lap_ms;       // Velocidad base del piloto (ms)
        uint32_t last_lap_ms;
        uint32_t best_lap_ms;
        uint16_t lap_count;
        uint32_t total_time_ms;
        uint32_t next_lap_at_ms;    // Timestamp para completar siguiente vuelta
    };

    PilotState pilots_[MAX_PILOTS];
    uint8_t    num_pilots_ = 0;
    bool       data_changed_ = false;

    // Recalcula posiciones basándose en vueltas y tiempo acumulado.
    void recalculate_positions(PilotEntry output[], uint8_t count);
};
```

### Módulo: Render Engine (`render.h` / `render.cpp`)

Responsable de dibujar el Race Dashboard con actualizaciones parciales.

```cpp
// render.h
#pragma once

#include <TFT_eSPI.h>
#include "data_provider.h"

namespace render {
    // Constantes de layout
    static const uint16_t SCREEN_W = 480;
    static const uint16_t SCREEN_H = 320;
    static const uint8_t  HEADER_H = 30;
    static const uint8_t  ROW_H    = 36;

    // Columnas: Pos | Nombre | Última | Mejor | Vueltas | Gap
    static const uint8_t NUM_COLS = 6;

    struct CellCache {
        char text[16];          // Texto renderizado previamente
        uint16_t fg_color;      // Color de texto usado
    };

    struct RenderState {
        CellCache cells[MAX_PILOTS][NUM_COLS];
        uint8_t   last_pilot_count;
        bool      header_drawn;
    };

    // Inicializa el estado de renderizado (limpia cache).
    void init_render(RenderState& state);

    // Dibuja el encabezado de la tabla (solo una vez o al reiniciar).
    void draw_header(TFT_eSPI& tft, RenderState& state);

    // Renderiza los datos de pilotos con partial update.
    // Solo redibuja celdas cuyos valores han cambiado.
    void render_dashboard(TFT_eSPI& tft, RenderState& state,
                          const PilotEntry pilots[], uint8_t count);
}
```

### Main Loop (`main.cpp`)

Orquesta la inicialización y el ciclo principal.

```cpp
// main.cpp (estructura conceptual)
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "display.h"
#include "simulation.h"
#include "render.h"

TFT_eSPI tft = TFT_eSPI();
SimulationEngine simulation;
render::RenderState render_state;

void setup() {
    if (!display::init_display(tft)) {
        display::signal_error();
        return;
    }
    display::show_splash_screen(tft);
    delay(2000);

    simulation.init(6);  // 6 pilotos
    render::init_render(render_state);
    render::draw_header(tft, render_state);
}

void loop() {
    simulation.update();

    if (simulation.has_new_data()) {
        PilotEntry pilots[MAX_PILOTS];
        uint8_t count = simulation.get_pilots(pilots, MAX_PILOTS);
        render::render_dashboard(tft, render_state, pilots, count);
    }
}
```

## Modelos de Datos

### PilotEntry

| Campo | Tipo | Descripción | Rango |
|-------|------|-------------|-------|
| `position` | `uint8_t` | Posición en la clasificación | 1-8 |
| `name` | `char[13]` | Nombre del piloto (null-terminated) | Hasta 12 caracteres |
| `last_lap_ms` | `uint32_t` | Tiempo de última vuelta en milisegundos | 10000-20000 (simulación) |
| `best_lap_ms` | `uint32_t` | Mejor tiempo de vuelta en milisegundos | 10000-20000 (simulación) |
| `lap_count` | `uint16_t` | Número total de vueltas completadas | 0-65535 |
| `gap_ms` | `int32_t` | Gap respecto al líder en ms. -1 indica líder | -1 o ≥0 |
| `total_time_ms` | `uint32_t` | Tiempo acumulado total en milisegundos | 0-4294967295 |

### Formato de Visualización

| Columna | Formato | Ejemplo | Ancho aprox. (px) |
|---------|---------|---------|-------------------|
| Posición | `"N"` | `"1"` | 30 |
| Nombre | Texto directo | `"PILOT_01"` | 120 |
| Última Vuelta | `"SS.sss"` | `"12.345"` | 90 |
| Mejor Vuelta | `"SS.sss"` | `"11.234"` | 90 |
| Vueltas | `"NN"` | `"15"` | 50 |
| Gap | `"+SS.sss"` o `"---"` | `"+01.234"` | 100 |

### Conversión de Milisegundos a Formato Display

```
Dado un tiempo en ms (ej: 12345):
  segundos = ms / 1000          → 12
  milisegundos = ms % 1000      → 345
  formato = sprintf("%2d.%03d", segundos, milisegundos) → "12.345"
```

### Esquema de Colores

| Elemento | Color RGB | Código TFT_eSPI |
|----------|-----------|-----------------|
| Fondo | (0, 0, 0) | `TFT_BLACK` |
| Texto base | (255, 255, 255) | `TFT_WHITE` |
| Posición líder | (0, 255, 0) | `TFT_GREEN` |
| Peor tiempo | (255, 0, 0) | `TFT_RED` |
| Encabezado | (128, 128, 128) | `TFT_DARKGREY` |

### Configuración de Hardware SPI

Basado en la documentación del [Seeed Studio XIAO ESP32-S3](https://wiki.seeedstudio.com/xiao_esp32s3_pin_multiplexing/) y configuraciones verificadas en la comunidad para ILI9488:

| Señal SPI | GPIO ESP32-S3 | Pin XIAO | Pin ILI9488 |
|-----------|---------------|----------|-------------|
| MOSI | GPIO10 (D10) | D10 | SDI (MOSI) |
| MISO | GPIO9 (D9) | D9 | SDO (MISO) |
| SCK | GPIO8 (D8) | D8 | SCK |
| CS | GPIO2 (D0) | D0 | CS |
| DC | GPIO4 (D2) | D2 | DC/RS |
| RST | GPIO5 (D3) | D3 | RESET |
| BL | GPIO3 (D1) | D1 | LED (Backlight) |

**Nota**: El ILI9488 opera en modo SPI de 18 bits (RGB666). TFT_eSPI maneja esta conversión internamente.

### Guía de Alimentación

#### Durante desarrollo (conectado a PC)

```
Tu PC ──USB-C──► XIAO ESP32-S3 ──pin 5V──► ILI9488 TFT VCC
                                 ──GND────► ILI9488 TFT GND
                                 ──SPI────► (MOSI, SCK, CS, DC, RST, BL)
```

- El XIAO ESP32-S3 recibe 5V directamente por el puerto USB-C desde la PC.
- El pin **5V** del XIAO es un passthrough del USB: saca los 5V sin regulación.
- El pin **3V3** del XIAO entrega 3.3V regulados internamente (no se usa para alimentar el TFT).
- El módulo TFT ILI9488 acepta 5V en su pin VCC (tiene regulador interno a 3.3V).
- **No se necesita fuente externa** durante desarrollo.

| Componente | Consumo típico |
|------------|---------------|
| XIAO ESP32-S3 | ~100 mA |
| ILI9488 TFT + backlight | ~150-200 mA |
| **Total** | **~300 mA** |

Un puerto USB 2.0 entrega hasta 500 mA y USB 3.0 hasta 900 mA, por lo que un puerto USB estándar es suficiente.

#### Modo standalone (sin PC)

Para uso en campo (por ejemplo en una pista FPV), se requiere una fuente externa:

```
Batería LiPo ──► Boost Converter (5V) ──► pin 5V del XIAO
                                        ──► GND del XIAO
```

- Usar un boost converter que entregue 5V estables con al menos 500 mA de capacidad.
- Conectar la salida del boost converter al pin **5V** y **GND** del XIAO.
- El XIAO alimentará la pantalla TFT a través de su pin 5V igual que en modo desarrollo.
- **No conectar USB y fuente externa simultáneamente** para evitar conflictos de alimentación.

## Propiedades de Corrección

*Una propiedad es una característica o comportamiento que debe cumplirse en todas las ejecuciones válidas de un sistema — esencialmente, una declaración formal sobre lo que el sistema debe hacer. Las propiedades sirven como puente entre especificaciones legibles por humanos y garantías de corrección verificables por máquina.*

### Propiedad 1: Número correcto de filas de pilotos

*Para cualquier* número de pilotos N en el rango [5, 8], al renderizar el dashboard, el número de filas de datos producidas debe ser exactamente N.

**Valida: Requisitos 2.1**

### Propiedad 2: Formato completo de PilotEntry

*Para cualquier* PilotEntry con valores arbitrarios válidos (last_lap_ms y best_lap_ms en [10000, 20000], lap_count ≥ 0, gap_ms ≥ -1), la cadena formateada para cada celda debe contener: la posición como número, el nombre del piloto, el tiempo de última vuelta en formato "SS.sss", el mejor tiempo en formato "SS.sss", el número de vueltas, y el gap en formato "+SS.sss" o "---" para el líder.

**Valida: Requisitos 2.3, 3.4**

### Propiedad 3: Asignación de colores por reglas

*Para cualquier* conjunto de pilotos con posiciones y tiempos de última vuelta distintos, el piloto en posición 1 debe recibir color verde (0,255,0) en su celda de posición, y el piloto con el mayor last_lap_ms debe recibir color rojo (255,0,0) en su celda de última vuelta.

**Valida: Requisitos 2.5, 2.6**

### Propiedad 4: Ordenamiento de posiciones por vueltas y tiempo

*Para cualquier* conjunto de pilotos con lap_count y total_time_ms arbitrarios, después de recalcular posiciones, la lista resultante debe estar ordenada de forma que: un piloto con más vueltas siempre precede a uno con menos vueltas, y entre pilotos con igual número de vueltas, el de menor tiempo acumulado precede al de mayor tiempo.

**Valida: Requisitos 3.1**

### Propiedad 5: Cálculo correcto del gap

*Para cualquier* conjunto de pilotos ya ordenados por posición, el gap del piloto en posición 1 debe ser -1 (líder), y para todo piloto en posición > 1, su gap_ms debe ser igual a su total_time_ms menos el total_time_ms del piloto en posición 1.

**Valida: Requisitos 3.3, 3.4**

### Propiedad 6: Inicialización correcta de la simulación

*Para cualquier* valor num_pilots en [5, 8], después de inicializar la simulación, debe haber exactamente num_pilots pilotos, cada uno con un nombre no vacío y una velocidad base (base_lap_ms) distinta de la de los demás pilotos.

**Valida: Requisitos 4.1, 4.5**

### Propiedad 7: Tiempos de vuelta dentro del rango

*Para cualquier* piloto y cualquier vuelta generada por la simulación, el tiempo de vuelta producido debe estar en el rango [10000, 20000] milisegundos.

**Valida: Requisitos 4.2**

### Propiedad 8: Invariantes de datos del piloto

*Para cualquier* estado de la simulación después de N actualizaciones (N ≥ 1), para cada piloto: best_lap_ms debe ser menor o igual a todos los tiempos de vuelta generados (incluido last_lap_ms), lap_count debe ser igual al número de vueltas completadas, y total_time_ms debe ser igual a la suma de todos los tiempos de vuelta individuales.

**Valida: Requisitos 4.4**

### Propiedad 9: Partial update solo redibuja celdas cambiadas

*Para cualquier* par de estados consecutivos de pilotos (estado anterior y estado actual), el Render_Engine debe redibujar únicamente las celdas cuyo texto o color ha cambiado entre ambos estados, y no redibujar las celdas que permanecen iguales.

**Valida: Requisitos 5.1**

### Propiedad 10: Flag de datos nuevos (round-trip)

*Para cualquier* secuencia de operaciones sobre un Data_Provider, después de llamar a get_pilots(), has_new_data() debe retornar false. Después de llamar a update() que produce un cambio, has_new_data() debe retornar true.

**Valida: Requisitos 6.4**

## Plan de Integración con FPVGate

### Sobre FPVGate

[FPVGate](https://github.com/LouisHitchcock/FPVGate) es un sistema de laptimer FPV basado en RSSI que usa el mismo microcontrolador (Seeed Studio XIAO ESP32-S3). Detecta la señal de video 5.8GHz de los drones para medir tiempos de vuelta. Es el sistema con el que este display se integrará en el futuro.

### Mecanismos de comunicación disponibles en FPVGate

| Mecanismo | Descripción | Viabilidad para el display |
|-----------|-------------|---------------------------|
| **WiFi AP** | FPVGate crea su propia red WiFi | ✅ El display se conecta como cliente WiFi |
| **HTTP Webhooks** | FPVGate envía eventos HTTP a dispositivos externos (race start/stop, laps) | ✅ Ideal — push de datos en tiempo real |
| **Interfaz Web** | Dashboard web accesible desde cualquier dispositivo en la red | ⚠️ Posible vía HTTP polling, pero menos eficiente |
| **USB Serial CDC** | Conexión directa por USB | ❌ Requiere cable físico entre ambos XIAO |

### Estrategia de integración recomendada

```
FPVGate (XIAO ESP32-S3)                    Display (XIAO ESP32-S3)
┌──────────────────────┐                    ┌──────────────────────┐
│                      │                    │                      │
│  WiFi Access Point   │◄───WiFi Client────│  FPVGateProvider     │
│  192.168.4.1         │                    │  (implementa         │
│                      │                    │   DataProvider)      │
│  HTTP Webhooks ──────┼───── POST ────────►│                      │
│  /api/race/status ◄──┼───── GET ─────────│  Polling fallback    │
│                      │                    │                      │
└──────────────────────┘                    └──────────────────────┘
```

**Fase 2A — Webhooks (preferido):**
1. El display se conecta al WiFi AP de FPVGate
2. Se registra como receptor de webhooks
3. FPVGate envía eventos HTTP POST al display cuando hay laps, start/stop de carrera
4. El display parsea el JSON y actualiza los datos de pilotos

**Fase 2B — HTTP Polling (fallback):**
1. El display se conecta al WiFi AP de FPVGate
2. Cada 500ms consulta un endpoint HTTP de FPVGate para obtener datos de carrera
3. Parsea la respuesta y actualiza los datos

### Implementación: clase FPVGateProvider

```cpp
// fpvgate_provider.h (futuro)
#pragma once

#include "data_provider.h"
#include <WiFi.h>

class FPVGateProvider : public DataProvider {
public:
    // Conecta al WiFi AP de FPVGate
    bool connect(const char* ssid, const char* password);

    void update() override;        // Recibe webhooks o hace polling
    bool has_new_data() const override;
    uint8_t get_pilots(PilotEntry pilots[], uint8_t max_count) override;

private:
    PilotEntry pilots_[MAX_PILOTS];
    uint8_t num_pilots_ = 0;
    bool data_changed_ = false;
    bool connected_ = false;
};
```

### Cambio en main.cpp para integración

```cpp
// Futuro: selección de fuente de datos
DataProvider* provider;

#ifdef USE_FPVGATE
    FPVGateProvider fpvgate;
    fpvgate.connect("FPVGate_AP", "password");
    provider = &fpvgate;
#else
    SimulationEngine simulation;
    simulation.init(6);
    provider = &simulation;
#endif

// El loop no cambia — usa la interfaz DataProvider
provider->update();
if (provider->has_new_data()) {
    PilotEntry pilots[MAX_PILOTS];
    uint8_t count = provider->get_pilots(pilots, MAX_PILOTS);
    render::render_dashboard(tft, render_state, pilots, count);
}
```

### Datos necesarios de FPVGate

Para llenar un `PilotEntry` desde FPVGate, necesitamos mapear:

| Campo PilotEntry | Dato de FPVGate | Notas |
|-----------------|-----------------|-------|
| `name` | Nombre del piloto configurado | Máx 12 caracteres |
| `last_lap_ms` | Último tiempo de vuelta registrado | En milisegundos |
| `best_lap_ms` | Mejor tiempo de vuelta | Calculado por FPVGate o localmente |
| `lap_count` | Número de vueltas completadas | Contador incremental |
| `gap_ms` | Diferencia con el líder | Calculado localmente con `race_logic` |
| `total_time_ms` | Tiempo acumulado | Suma de todos los tiempos de vuelta |

### Requisitos pendientes para Fase 2

- Investigar el formato exacto de los webhooks de FPVGate (JSON payload)
- Determinar si FPVGate expone un endpoint REST para polling
- Definir manejo de reconexión WiFi si se pierde la señal
- Definir UI para mostrar estado de conexión WiFi en el dashboard

## Manejo de Errores

### Errores de Inicialización

| Error | Detección | Respuesta |
|-------|-----------|-----------|
| Fallo de inicialización TFT | `tft.init()` no responde o retorna error | LED integrado parpadea a 5 Hz indefinidamente. El sistema no continúa al loop principal. |
| Configuración SPI incorrecta | Pantalla en blanco tras init | No detectable por software; requiere verificación visual del cableado. |

### Errores en Tiempo de Ejecución

| Error | Detección | Respuesta |
|-------|-----------|-----------|
| Datos de simulación fuera de rango | Validación en `update()` | Clamp de valores al rango válido [10000, 20000] ms. Log por Serial si disponible. |
| Overflow de `total_time_ms` | `uint32_t` overflow (~49.7 días) | No se espera en uso normal. Para carreras largas, se podría resetear la simulación. |
| Número de pilotos fuera de rango | Validación en `init()` | Clamp a [5, 8]. |

### Estrategia de Robustez

- **Defensive programming**: Todos los valores de entrada se validan y se limitan (clamp) a rangos válidos antes de procesarlos.
- **Sin excepciones**: El firmware Arduino/C++ no usa excepciones. Los errores se manejan con valores de retorno y validación de rangos.
- **Watchdog**: Se puede habilitar el watchdog timer del ESP32-S3 para reiniciar el sistema si el loop principal se bloquea.

## Estrategia de Testing

### Enfoque Dual de Testing

El proyecto utiliza dos estrategias complementarias:

1. **Tests unitarios (example-based)**: Verifican escenarios específicos, casos borde y condiciones de error.
2. **Tests basados en propiedades (property-based)**: Verifican propiedades universales con entradas generadas aleatoriamente.

### Framework de Testing

- **Plataforma**: Tests ejecutados en el host (PC), no en el microcontrolador, usando compilación nativa.
- **Framework de tests unitarios**: Google Test (gtest) — estándar para C++, compatible con PlatformIO native.
- **Framework de property-based testing**: [RapidCheck](https://github.com/emil-e/rapidcheck) — biblioteca PBT para C++ que se integra con Google Test.
- **Configuración PBT**: Mínimo 100 iteraciones por test de propiedad.

### Qué se Testea en Host vs. Hardware

| Componente | Host (PC) | Hardware (ESP32) |
|------------|-----------|------------------|
| Formateo de tiempos (ms → "SS.sss") | ✅ Unit + PBT | — |
| Ordenamiento de posiciones | ✅ PBT | — |
| Cálculo de gaps | ✅ PBT | — |
| Asignación de colores | ✅ PBT | — |
| Lógica de simulación | ✅ PBT | — |
| Detección de cambios (partial update) | ✅ PBT | — |
| Flag has_new_data | ✅ PBT | — |
| Inicialización TFT | — | ✅ Smoke test manual |
| Renderizado visual | — | ✅ Verificación visual |
| Tasa de refresco | — | ✅ Medición con millis() |

### Estructura de Tests

```
test/
├── test_native/
│   ├── test_formatting.cpp      # Tests de formato SS.sss y gap
│   ├── test_sorting.cpp         # Tests de ordenamiento de posiciones
│   ├── test_simulation.cpp      # Tests del motor de simulación
│   ├── test_color_rules.cpp     # Tests de asignación de colores
│   ├── test_partial_update.cpp  # Tests de detección de cambios
│   └── test_data_provider.cpp   # Tests del flag has_new_data
```

### Mapeo de Propiedades a Tests

Cada test de propiedad debe incluir un comentario de referencia con el formato:

```
// Feature: fpv-laptimer-display, Property N: [texto de la propiedad]
```

| Propiedad | Archivo de Test | Tipo |
|-----------|----------------|------|
| P1: Número correcto de filas | test_formatting.cpp | PBT |
| P2: Formato completo de PilotEntry | test_formatting.cpp | PBT |
| P3: Asignación de colores | test_color_rules.cpp | PBT |
| P4: Ordenamiento de posiciones | test_sorting.cpp | PBT |
| P5: Cálculo de gaps | test_sorting.cpp | PBT |
| P6: Inicialización de simulación | test_simulation.cpp | PBT |
| P7: Tiempos en rango | test_simulation.cpp | PBT |
| P8: Invariantes de datos | test_simulation.cpp | PBT |
| P9: Partial update selectivo | test_partial_update.cpp | PBT |
| P10: Flag de datos nuevos | test_data_provider.cpp | PBT |

### Tests Unitarios Complementarios (Example-Based)

Además de los tests de propiedades, se incluyen tests unitarios para:

- **Caso borde**: Gap del líder se muestra como "---" (Req 3.4)
- **Caso borde**: Piloto con 0 vueltas completadas
- **Ejemplo**: Encabezado contiene todas las columnas requeridas (Req 2.2)
- **Ejemplo**: Colores base son negro/blanco (Req 2.4)
- **Ejemplo**: Layout ocupa 480x320 (Req 2.7)
- **Ejemplo**: Secuencia clear-then-draw en partial update (Req 5.4)
- **Smoke**: platformio.ini tiene configuración correcta (Req 7.2, 8.2)
- **Smoke**: User_Setup.h tiene pines correctos (Req 7.4)

