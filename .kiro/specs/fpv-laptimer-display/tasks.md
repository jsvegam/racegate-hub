# Plan de Implementación: FPV Laptimer Display

## Resumen

Implementación incremental de un display standalone de laptimer FPV usando un Seeed Studio XIAO ESP32-S3 con pantalla ILI9488 3.5" (480x320). El plan sigue el orden: configuración del proyecto → estructuras de datos e interfaces → simulación → renderizado → integración en main.cpp. Los tests se ejecutan en el host (PC) con Google Test y RapidCheck usando compilación nativa de PlatformIO.

## Tareas

- [x] 1. Configurar proyecto PlatformIO y estructura de archivos
  - [x] 1.1 Crear `platformio.ini` con dos entornos: `esp32s3` (target) y `native` (tests en host)
    - Entorno `esp32s3`: placa `seeed_xiao_esp32s3`, framework `arduino`, dependencia `TFT_eSPI`
    - Incluir build flags para configurar TFT_eSPI: driver ILI9488, pines SPI (MOSI=GPIO10, MISO=GPIO9, SCK=GPIO8, CS=GPIO2, DC=GPIO4, RST=GPIO5, BL=GPIO3), resolución 480x320
    - Entorno `native`: plataforma `native`, dependencias `googletest` y `rapidcheck`, build flags para compilación en host
    - _Requisitos: 7.2, 7.3, 7.4, 8.2_
  - [x] 1.2 Crear la estructura de directorios del proyecto
    - Crear `src/` con archivos vacíos: `main.cpp`, `display.h`, `display.cpp`, `data_provider.h`, `simulation.h`, `simulation.cpp`, `render.h`, `render.cpp`
    - Crear `test/test_native/` con archivos vacíos para los tests
    - Crear `README.md` con documentación del mapeo de pines SPI y guía de compilación/flasheo
    - _Requisitos: 7.1, 8.1, 8.3_

- [x] 2. Implementar estructuras de datos e interfaz DataProvider
  - [x] 2.1 Implementar `data_provider.h` con `PilotEntry` y clase abstracta `DataProvider`
    - Definir constantes `MAX_PILOTS = 8` y `NAME_MAX_LEN = 12`
    - Definir struct `PilotEntry` con campos: `position` (uint8_t), `name` (char[13]), `last_lap_ms` (uint32_t), `best_lap_ms` (uint32_t), `lap_count` (uint16_t), `gap_ms` (int32_t), `total_time_ms` (uint32_t)
    - Definir clase abstracta `DataProvider` con métodos virtuales puros: `update()`, `has_new_data()`, `get_pilots()`
    - _Requisitos: 6.1, 6.4_
  - [x] 2.2 Implementar funciones auxiliares de formateo en `render.h`/`render.cpp`
    - Función para convertir milisegundos a formato "SS.sss" (ej: 12345 → "12.345")
    - Función para formatear gap: -1 → "---", otros → "+SS.sss"
    - Definir constantes de layout: `SCREEN_W=480`, `SCREEN_H=320`, `HEADER_H=30`, `ROW_H=36`, `NUM_COLS=6`
    - Definir structs `CellCache` y `RenderState`
    - _Requisitos: 2.3, 2.7, 3.4_
  - [ ]* 2.3 Escribir test de propiedad para número correcto de filas
    - **Propiedad 1: Número correcto de filas de pilotos**
    - Para cualquier N en [5,8], al generar N pilotos, el conteo de filas de datos debe ser exactamente N
    - **Valida: Requisito 2.1**
  - [ ]* 2.4 Escribir test de propiedad para formato completo de PilotEntry
    - **Propiedad 2: Formato completo de PilotEntry**
    - Para cualquier PilotEntry con valores válidos, la cadena formateada debe contener posición, nombre, tiempos en "SS.sss", vueltas, y gap en "+SS.sss" o "---"
    - **Valida: Requisitos 2.3, 3.4**

- [x] 3. Implementar lógica de ordenamiento y cálculo de gaps
  - [x] 3.1 Implementar función de ordenamiento de posiciones
    - Ordenar pilotos por: (1) mayor `lap_count` primero, (2) menor `total_time_ms` como desempate
    - Asignar campo `position` (1-N) según el orden resultante
    - Implementar en un módulo accesible tanto por SimulationEngine como por tests nativos
    - _Requisitos: 3.1, 3.2_
  - [x] 3.2 Implementar función de cálculo de gaps
    - Gap del líder (posición 1) = -1
    - Gap de otros pilotos = `total_time_ms` del piloto - `total_time_ms` del líder
    - _Requisitos: 3.3, 3.4_
  - [ ]* 3.3 Escribir test de propiedad para ordenamiento de posiciones
    - **Propiedad 4: Ordenamiento de posiciones por vueltas y tiempo**
    - Para cualquier conjunto de pilotos, después de ordenar: más vueltas → posición menor; mismas vueltas → menor tiempo acumulado → posición menor
    - **Valida: Requisito 3.1**
  - [ ]* 3.4 Escribir test de propiedad para cálculo de gaps
    - **Propiedad 5: Cálculo correcto del gap**
    - Para cualquier conjunto ordenado, gap del líder = -1, gap de otros = su total_time_ms - total_time_ms del líder
    - **Valida: Requisitos 3.3, 3.4**

- [x] 4. Checkpoint — Verificar que los tests pasan
  - Asegurar que todos los tests del entorno `native` pasan correctamente. Preguntar al usuario si surgen dudas.

- [x] 5. Implementar SimulationEngine
  - [x] 5.1 Implementar `simulation.h` y `simulation.cpp`
    - Método `init(num_pilots)`: crear entre 5-8 pilotos con nombres predefinidos, asignar velocidad base distinta a cada piloto dentro del rango [10000, 20000] ms
    - Método `update()`: usar `millis()` para determinar cuándo un piloto completa una vuelta (cada 2-4 segundos), generar tiempo de vuelta aleatorio basado en velocidad base ± variación, actualizar `last_lap_ms`, `best_lap_ms`, `lap_count`, `total_time_ms`, recalcular posiciones y gaps, marcar `data_changed_ = true`
    - Método `has_new_data()`: retornar `data_changed_`
    - Método `get_pilots()`: llenar array de PilotEntry ordenados, resetear `data_changed_ = false`
    - Validar y clampear valores fuera de rango
    - _Requisitos: 4.1, 4.2, 4.3, 4.4, 4.5, 6.2_
  - [ ]* 5.2 Escribir test de propiedad para inicialización de simulación
    - **Propiedad 6: Inicialización correcta de la simulación**
    - Para cualquier num_pilots en [5,8], después de init: exactamente num_pilots pilotos, cada uno con nombre no vacío y velocidad base distinta
    - **Valida: Requisitos 4.1, 4.5**
  - [ ]* 5.3 Escribir test de propiedad para tiempos de vuelta en rango
    - **Propiedad 7: Tiempos de vuelta dentro del rango**
    - Para cualquier piloto y vuelta generada, el tiempo debe estar en [10000, 20000] ms
    - **Valida: Requisito 4.2**
  - [ ]* 5.4 Escribir test de propiedad para invariantes de datos del piloto
    - **Propiedad 8: Invariantes de datos del piloto**
    - Después de N actualizaciones: best_lap_ms ≤ todos los tiempos generados, lap_count = número de vueltas completadas, total_time_ms = suma de tiempos individuales
    - **Valida: Requisito 4.4**
  - [ ]* 5.5 Escribir test de propiedad para flag de datos nuevos
    - **Propiedad 10: Flag de datos nuevos (round-trip)**
    - Después de get_pilots(), has_new_data() retorna false. Después de update() con cambio, has_new_data() retorna true
    - **Valida: Requisito 6.4**

- [x] 6. Checkpoint — Verificar tests de simulación
  - Asegurar que todos los tests del entorno `native` pasan correctamente. Preguntar al usuario si surgen dudas.

- [x] 7. Implementar Render Engine con partial update
  - [x] 7.1 Implementar lógica de asignación de colores en `render.cpp`
    - Posición 1 (líder): texto de posición en verde `TFT_GREEN` (0,255,0)
    - Peor tiempo de última vuelta: texto en rojo `TFT_RED` (255,0,0)
    - Texto base: blanco `TFT_WHITE` sobre fondo negro `TFT_BLACK`
    - Encabezado: gris `TFT_DARKGREY`
    - _Requisitos: 2.4, 2.5, 2.6_
  - [x] 7.2 Implementar `init_render()` y `draw_header()` en `render.cpp`
    - `init_render()`: limpiar CellCache, resetear `last_pilot_count` y `header_drawn`
    - `draw_header()`: dibujar fila de encabezado con columnas "Pos", "Nombre", "Última", "Mejor", "Vueltas", "Gap"
    - _Requisitos: 2.2, 2.7_
  - [x] 7.3 Implementar `render_dashboard()` con partial update
    - Para cada piloto y cada columna: formatear el valor actual, comparar con `CellCache`
    - Si el texto o color cambió: borrar celda con `fillRect(TFT_BLACK)`, escribir nuevo texto, actualizar cache
    - Si no cambió: no redibujar (partial update)
    - Si el número de pilotos disminuyó: limpiar filas sobrantes
    - _Requisitos: 5.1, 5.3, 5.4_
  - [ ]* 7.4 Escribir test de propiedad para asignación de colores
    - **Propiedad 3: Asignación de colores por reglas**
    - Para cualquier conjunto de pilotos: posición 1 → verde en celda de posición, mayor last_lap_ms → rojo en celda de última vuelta
    - **Valida: Requisitos 2.5, 2.6**
  - [ ]* 7.5 Escribir test de propiedad para partial update
    - **Propiedad 9: Partial update solo redibuja celdas cambiadas**
    - Para cualquier par de estados consecutivos, solo se redibujan celdas cuyo texto o color cambió
    - **Valida: Requisito 5.1**

- [x] 8. Implementar módulo Display y main.cpp
  - [x] 8.1 Implementar `display.h` y `display.cpp`
    - `init_display(tft)`: inicializar TFT_eSPI, configurar rotación para 480x320, encender backlight (GPIO3), retornar true/false
    - `show_splash_screen(tft)`: mostrar nombre del sistema centrado en pantalla durante máximo 2 segundos
    - `signal_error()`: parpadear LED integrado (GPIO21) a 5 Hz indefinidamente
    - _Requisitos: 1.1, 1.2, 1.3, 1.4_
  - [x] 8.2 Implementar `main.cpp` con setup() y loop()
    - `setup()`: llamar `init_display()`, si falla llamar `signal_error()` y retornar, mostrar splash screen con delay de 2s, inicializar SimulationEngine con 6 pilotos, inicializar RenderState, dibujar encabezado
    - `loop()`: llamar `simulation.update()`, si `has_new_data()` obtener pilotos y llamar `render_dashboard()`
    - _Requisitos: 1.1, 1.2, 1.3, 6.3, 7.1_

- [x] 9. Checkpoint final — Compilación y verificación completa
  - Compilar el entorno `esp32s3` con `pio run -e esp32s3` para verificar que no hay errores de compilación
  - Ejecutar todos los tests nativos con `pio test -e native`
  - Asegurar que todos los tests pasan. Preguntar al usuario si surgen dudas.

## Notas

- Las tareas marcadas con `*` son opcionales y pueden omitirse para un MVP más rápido.
- Cada tarea referencia los requisitos específicos que cubre para trazabilidad.
- Los checkpoints aseguran validación incremental del progreso.
- Los tests de propiedades validan propiedades universales de corrección definidas en el diseño.
- Los tests unitarios complementarios validan ejemplos específicos y casos borde.
- Los tests se ejecutan en el host (PC) con compilación nativa; la verificación visual se hace en el hardware.
