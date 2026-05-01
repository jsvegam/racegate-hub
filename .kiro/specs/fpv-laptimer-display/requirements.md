# Documento de Requisitos — FPV Laptimer Display

## Introducción

Este documento define los requisitos para un display standalone de laptimer FPV. El sistema utiliza un microcontrolador ESP32-S3 (Seeed Studio XIAO ESP32-S3) conectado a una pantalla TFT ILI9488 de 3.5" (480x320) vía SPI. El display muestra un dashboard de carrera en tiempo real con datos de pilotos, tiempos de vuelta, posiciones y gaps. En su fase inicial, el sistema opera de forma autónoma con datos simulados, pero su arquitectura permite reemplazar la fuente de datos por una conexión WiFi a un sistema FPVGate.

## Glosario

- **Display_System**: El sistema completo compuesto por el ESP32-S3, la pantalla TFT ILI9488 y el firmware que los controla.
- **Render_Engine**: El módulo de software responsable de dibujar la interfaz gráfica en la pantalla TFT.
- **Simulation_Engine**: El módulo de software que genera datos de carrera simulados (pilotos, tiempos de vuelta, posiciones).
- **Data_Provider**: La interfaz abstracta que suministra datos de carrera al Render_Engine, implementada por el Simulation_Engine o por un futuro cliente WiFi.
- **Pilot_Entry**: Un registro de datos de un piloto que incluye nombre, tiempo de última vuelta, mejor tiempo, número de vueltas, gap al líder y posición.
- **Race_Dashboard**: La interfaz visual tipo tabla que muestra las Pilot_Entry ordenadas por posición.
- **Partial_Update**: Técnica de renderizado que redibuja únicamente las celdas de la pantalla cuyos valores han cambiado, en lugar de redibujar toda la pantalla.
- **TFT_Display**: La pantalla TFT ILI9488 de 3.5" con resolución 480x320 píxeles conectada vía SPI.
- **SPI_Bus**: El bus de comunicación Serial Peripheral Interface utilizado para transmitir datos entre el ESP32-S3 y el TFT_Display.
- **FPVGate**: Sistema externo de laptimer FPV con el que el Display_System se integrará en el futuro vía WiFi.

## Requisitos

### Requisito 1: Inicialización del Hardware

**User Story:** Como operador de carrera, quiero que el display se inicialice correctamente al encender, para poder comenzar a visualizar datos de carrera de forma inmediata.

#### Criterios de Aceptación

1. WHEN el Display_System recibe alimentación, THE Display_System SHALL inicializar el SPI_Bus con la configuración de pines correspondiente al Seeed Studio XIAO ESP32-S3.
2. WHEN el SPI_Bus está inicializado, THE Display_System SHALL configurar el TFT_Display con el driver ILI9488 a una resolución de 480x320 píxeles.
3. WHEN el TFT_Display está configurado, THE Display_System SHALL mostrar una pantalla de inicio con el nombre del sistema durante un máximo de 2 segundos.
4. IF la inicialización del TFT_Display falla, THEN THE Display_System SHALL indicar el error mediante el LED integrado del ESP32-S3 con un patrón de parpadeo rápido (5 Hz).

### Requisito 2: Estructura del Race Dashboard

**User Story:** Como operador de carrera, quiero ver un dashboard tipo tabla con la información de todos los pilotos, para poder seguir el estado de la carrera de un vistazo.

#### Criterios de Aceptación

1. THE Race_Dashboard SHALL mostrar entre 5 y 8 Pilot_Entry simultáneamente en formato de tabla.
2. THE Race_Dashboard SHALL incluir una fila de encabezado con las columnas: Posición, Nombre, Última Vuelta, Mejor Vuelta, Vueltas, Gap.
3. THE Race_Dashboard SHALL mostrar cada Pilot_Entry en una fila que contenga: posición actual, nombre del piloto, tiempo de última vuelta en formato "SS.sss", mejor tiempo de vuelta en formato "SS.sss", número total de vueltas completadas, y gap respecto al líder en formato "+SS.sss".
4. THE Race_Dashboard SHALL utilizar un tema oscuro con fondo negro (RGB 0,0,0) y texto blanco (RGB 255,255,255) como colores base.
5. THE Race_Dashboard SHALL resaltar la Pilot_Entry del líder (posición 1) con color verde (RGB 0,255,0) en el texto de posición.
6. THE Race_Dashboard SHALL resaltar el peor tiempo de última vuelta entre todos los pilotos activos con color rojo (RGB 255,0,0).
7. THE Race_Dashboard SHALL ocupar la resolución completa de 480x320 píxeles del TFT_Display.

### Requisito 3: Ordenamiento Dinámico de Posiciones

**User Story:** Como operador de carrera, quiero que las posiciones se actualicen automáticamente según el progreso de la carrera, para ver siempre la clasificación correcta.

#### Criterios de Aceptación

1. WHEN una Pilot_Entry completa una nueva vuelta, THE Display_System SHALL recalcular las posiciones de todas las Pilot_Entry basándose en el número total de vueltas completadas como criterio primario y el menor tiempo acumulado como criterio secundario.
2. WHEN las posiciones cambian, THE Race_Dashboard SHALL reordenar las filas de la tabla para reflejar las nuevas posiciones.
3. THE Display_System SHALL calcular el gap de cada Pilot_Entry como la diferencia de tiempo acumulado respecto a la Pilot_Entry en posición 1.
4. THE Display_System SHALL mostrar el gap del líder como "---" en lugar de "+0.000".

### Requisito 4: Motor de Simulación de Datos

**User Story:** Como desarrollador, quiero un motor de simulación que genere datos de carrera realistas, para poder probar y demostrar el display sin necesidad de hardware de laptimer externo.

#### Criterios de Aceptación

1. WHEN el Display_System arranca en modo standalone, THE Simulation_Engine SHALL generar entre 5 y 8 pilotos con nombres predefinidos.
2. THE Simulation_Engine SHALL producir tiempos de vuelta aleatorios en el rango de 10.000 a 20.000 segundos para cada piloto.
3. THE Simulation_Engine SHALL completar una nueva vuelta para un piloto aleatorio cada 2 a 4 segundos.
4. THE Simulation_Engine SHALL mantener para cada piloto: el mejor tiempo de vuelta, el tiempo de la última vuelta, el número total de vueltas completadas y el tiempo acumulado total.
5. THE Simulation_Engine SHALL asignar a cada piloto una velocidad base diferente dentro del rango de tiempos, para producir variación natural en las posiciones.

### Requisito 5: Renderizado Eficiente

**User Story:** Como operador de carrera, quiero que la pantalla se actualice de forma fluida y sin parpadeos, para poder leer los datos cómodamente.

#### Criterios de Aceptación

1. THE Render_Engine SHALL utilizar Partial_Update para redibujar únicamente las celdas del Race_Dashboard cuyos valores han cambiado desde el último ciclo de renderizado.
2. THE Render_Engine SHALL mantener una tasa de refresco entre 5 y 10 cuadros por segundo.
3. THE Render_Engine SHALL completar cada ciclo de renderizado sin producir parpadeo visible en el TFT_Display.
4. WHEN un valor de una celda cambia, THE Render_Engine SHALL borrar el área de la celda con el color de fondo antes de escribir el nuevo valor, para evitar superposición de texto.

### Requisito 6: Abstracción de la Fuente de Datos

**User Story:** Como desarrollador, quiero que la fuente de datos esté abstraída detrás de una interfaz, para poder reemplazar la simulación por datos reales de FPVGate sin modificar el código de renderizado.

#### Criterios de Aceptación

1. THE Display_System SHALL definir una interfaz Data_Provider que exponga los datos de carrera (lista de Pilot_Entry con todos sus campos) de forma independiente de la fuente de datos.
2. THE Simulation_Engine SHALL implementar la interfaz Data_Provider.
3. THE Render_Engine SHALL obtener los datos de carrera exclusivamente a través de la interfaz Data_Provider, sin depender directamente del Simulation_Engine.
4. THE Data_Provider SHALL exponer un método para consultar si hay datos nuevos disponibles desde la última consulta.

### Requisito 7: Estructura Modular del Código

**User Story:** Como desarrollador, quiero que el código esté organizado en módulos claros, para facilitar el mantenimiento y la futura integración con FPVGate.

#### Criterios de Aceptación

1. THE Display_System SHALL organizar el código fuente en módulos separados: módulo de display (inicialización del hardware), módulo de simulación (Simulation_Engine), módulo de renderizado (Render_Engine) y bucle principal.
2. THE Display_System SHALL ser compatible con el sistema de compilación PlatformIO para ESP32-S3.
3. THE Display_System SHALL utilizar la biblioteca TFT_eSPI para la comunicación con el TFT_Display.
4. THE Display_System SHALL incluir un archivo de configuración de TFT_eSPI (User_Setup.h o equivalente) con los pines SPI correctos para el Seeed Studio XIAO ESP32-S3.

### Requisito 8: Configuración de Hardware y Documentación

**User Story:** Como desarrollador, quiero documentación clara del cableado y la configuración, para poder replicar el montaje del hardware correctamente.

#### Criterios de Aceptación

1. THE Display_System SHALL incluir documentación del mapeo de pines SPI entre el Seeed Studio XIAO ESP32-S3 y el TFT_Display ILI9488 (MOSI, MISO, SCK, CS, DC, RST, BL).
2. THE Display_System SHALL incluir un archivo platformio.ini configurado para la placa Seeed Studio XIAO ESP32-S3 con las dependencias necesarias.
3. THE Display_System SHALL incluir instrucciones para compilar y flashear el firmware mediante PlatformIO.
