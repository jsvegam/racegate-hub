#include "race_logic.h"
#include <algorithm>

namespace race_logic {

void sort_pilots(PilotEntry pilots[], uint8_t count) {
    if (count == 0) {
        return;
    }

    // Sort using std::sort with a comparator:
    // (1) Higher lap_count first
    // (2) Lower total_time_ms as tiebreaker
    std::sort(pilots, pilots + count, [](const PilotEntry& a, const PilotEntry& b) {
        if (a.lap_count != b.lap_count) {
            return a.lap_count > b.lap_count;  // More laps = better position
        }
        return a.total_time_ms < b.total_time_ms;  // Less time = better position
    });

    // Assign positions 1..N
    for (uint8_t i = 0; i < count; ++i) {
        pilots[i].position = i + 1;
    }
}

void calculate_gaps(PilotEntry pilots[], uint8_t count) {
    if (count == 0) {
        return;
    }

    // Leader (index 0, position 1) gets gap = -1
    pilots[0].gap_ms = -1;

    // Others get gap = their total_time_ms - leader's total_time_ms
    uint32_t leader_time = pilots[0].total_time_ms;
    for (uint8_t i = 1; i < count; ++i) {
        pilots[i].gap_ms = static_cast<int32_t>(pilots[i].total_time_ms - leader_time);
    }
}

} // namespace race_logic
