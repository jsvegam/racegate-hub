#pragma once

#include "data_provider.h"
#include "race_logic.h"
#include <cstdlib>

#ifdef NATIVE_BUILD
#include <cstdint>
#else
#include <Arduino.h>
#endif

class SimulationEngine : public DataProvider {
public:
    void init(uint8_t num_pilots);

    // Overload for testability: accepts current time directly
    void update(uint32_t current_time_ms);

    void update() override;
    bool has_new_data() const override;
    uint8_t get_pilots(PilotEntry pilots[], uint8_t max_count) override;

    // --- Accessors for testing ---
    uint8_t num_pilots() const { return num_pilots_; }

    struct PilotState {
        char     name[NAME_MAX_LEN + 1];
        uint32_t base_lap_ms;
        uint32_t last_lap_ms;
        uint32_t best_lap_ms;
        uint16_t lap_count;
        uint32_t total_time_ms;
        uint32_t next_lap_at_ms;
    };

    const PilotState& pilot_state(uint8_t index) const { return pilots_[index]; }

    void set_random_seed(unsigned int seed) { random_seed_ = seed; }

private:
    static const char* const PILOT_NAMES[];
    static const uint8_t NUM_PREDEFINED_NAMES;

    PilotState pilots_[MAX_PILOTS];
    uint8_t    num_pilots_ = 0;
    bool       data_changed_ = false;
    unsigned int random_seed_ = 42;

    uint32_t random_range(uint32_t min_val, uint32_t max_val);
    uint32_t generate_lap_time(uint8_t pilot_index);
    void schedule_next_lap(uint8_t pilot_index, uint32_t current_time_ms);
};
