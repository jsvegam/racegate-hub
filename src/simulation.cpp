#include "simulation.h"
#include <cstring>
#include <algorithm>

const char* const SimulationEngine::PILOT_NAMES[] = {
    "RAZOR", "VIPER", "GHOST", "BLITZ", "STORM", "PHOENIX", "SHADOW", "TURBO"
};
const uint8_t SimulationEngine::NUM_PREDEFINED_NAMES = 8;

static const uint32_t MIN_LAP_MS = 10000;
static const uint32_t MAX_LAP_MS = 20000;
static const uint32_t MIN_INTERVAL_MS = 2000;
static const uint32_t MAX_INTERVAL_MS = 4000;

uint32_t SimulationEngine::random_range(uint32_t min_val, uint32_t max_val) {
    if (min_val >= max_val) return min_val;
    random_seed_ = random_seed_ * 1103515245u + 12345u;
    uint32_t raw = (random_seed_ >> 16) & 0x7FFF;
    return min_val + (raw % (max_val - min_val + 1));
}

uint32_t SimulationEngine::generate_lap_time(uint8_t pilot_index) {
    uint32_t base = pilots_[pilot_index].base_lap_ms;
    uint32_t variation = base * 15 / 100;
    uint32_t lo = (base > variation) ? (base - variation) : MIN_LAP_MS;
    uint32_t hi = base + variation;
    if (lo < MIN_LAP_MS) lo = MIN_LAP_MS;
    if (hi > MAX_LAP_MS) hi = MAX_LAP_MS;
    if (lo > hi) lo = hi;

    uint32_t lap = random_range(lo, hi);
    if (lap < MIN_LAP_MS) lap = MIN_LAP_MS;
    if (lap > MAX_LAP_MS) lap = MAX_LAP_MS;
    return lap;
}

void SimulationEngine::schedule_next_lap(uint8_t pilot_index, uint32_t now) {
    pilots_[pilot_index].next_lap_at_ms = now + random_range(MIN_INTERVAL_MS, MAX_INTERVAL_MS);
}

void SimulationEngine::init(uint8_t num_pilots) {
    if (num_pilots < 5) num_pilots = 5;
    if (num_pilots > 8) num_pilots = 8;
    num_pilots_ = num_pilots;
    data_changed_ = false;

    uint32_t step = (num_pilots > 1)
        ? (MAX_LAP_MS - MIN_LAP_MS) / (num_pilots - 1)
        : 0;

    for (uint8_t i = 0; i < num_pilots_; ++i) {
        std::memset(&pilots_[i], 0, sizeof(PilotState));
        std::strncpy(pilots_[i].name, PILOT_NAMES[i], NAME_MAX_LEN);
        pilots_[i].name[NAME_MAX_LEN] = '\0';
        pilots_[i].base_lap_ms = MIN_LAP_MS + (step * i);
        schedule_next_lap(i, 0);
    }
}

void SimulationEngine::update(uint32_t current_time_ms) {
    bool any_change = false;
    for (uint8_t i = 0; i < num_pilots_; ++i) {
        if (current_time_ms >= pilots_[i].next_lap_at_ms) {
            uint32_t lap = generate_lap_time(i);
            pilots_[i].last_lap_ms = lap;
            pilots_[i].lap_count++;
            pilots_[i].total_time_ms += lap;
            if (pilots_[i].best_lap_ms == 0 || lap < pilots_[i].best_lap_ms) {
                pilots_[i].best_lap_ms = lap;
            }
            schedule_next_lap(i, current_time_ms);
            any_change = true;
        }
    }
    if (any_change) data_changed_ = true;
}

void SimulationEngine::update() {
#ifndef NATIVE_BUILD
    update(millis());
#endif
}

bool SimulationEngine::has_new_data() const {
    return data_changed_;
}

uint8_t SimulationEngine::get_pilots(PilotEntry pilots[], uint8_t max_count) {
    uint8_t count = (num_pilots_ < max_count) ? num_pilots_ : max_count;
    for (uint8_t i = 0; i < count; ++i) {
        pilots[i].position = 0;
        std::strncpy(pilots[i].name, pilots_[i].name, NAME_MAX_LEN);
        pilots[i].name[NAME_MAX_LEN] = '\0';
        pilots[i].last_lap_ms = pilots_[i].last_lap_ms;
        pilots[i].best_lap_ms = pilots_[i].best_lap_ms;
        pilots[i].lap_count = pilots_[i].lap_count;
        pilots[i].total_time_ms = pilots_[i].total_time_ms;
        pilots[i].gap_ms = 0;
    }
    race_logic::sort_pilots(pilots, count);
    race_logic::calculate_gaps(pilots, count);
    data_changed_ = false;
    return count;
}
