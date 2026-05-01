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
