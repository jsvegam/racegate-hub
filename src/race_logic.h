#pragma once

#include "data_provider.h"

namespace race_logic {

    // Sorts pilots by: (1) higher lap_count first, (2) lower total_time_ms as tiebreaker.
    // Assigns position field (1-N) based on resulting order.
    void sort_pilots(PilotEntry pilots[], uint8_t count);

    // Calculates gap_ms for each pilot.
    // Assumes pilots are already sorted by position (index 0 = leader).
    // Leader (position 1) gets gap_ms = -1.
    // Others get gap_ms = their total_time_ms - leader's total_time_ms.
    void calculate_gaps(PilotEntry pilots[], uint8_t count);

} // namespace race_logic
