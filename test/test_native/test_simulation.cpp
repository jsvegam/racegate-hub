#include <gtest/gtest.h>
#include "simulation.h"
#include <cstring>
#include <set>

// ─── Initialization tests ────────────────────────────────────────

TEST(SimulationInit, ClampsMinPilots) {
    SimulationEngine sim;
    sim.init(3);  // Below minimum 5
    EXPECT_EQ(sim.num_pilots(), 5);
}

TEST(SimulationInit, ClampsMaxPilots) {
    SimulationEngine sim;
    sim.init(12);  // Above maximum 8
    EXPECT_EQ(sim.num_pilots(), 8);
}

TEST(SimulationInit, AcceptsValidRange) {
    for (uint8_t n = 5; n <= 8; ++n) {
        SimulationEngine sim;
        sim.init(n);
        EXPECT_EQ(sim.num_pilots(), n);
    }
}

TEST(SimulationInit, PilotsHaveNonEmptyNames) {
    SimulationEngine sim;
    sim.init(8);
    for (uint8_t i = 0; i < 8; ++i) {
        EXPECT_GT(std::strlen(sim.pilot_state(i).name), 0u)
            << "Pilot " << (int)i << " has empty name";
    }
}

TEST(SimulationInit, PilotsHaveDistinctBaseSpeeds) {
    SimulationEngine sim;
    sim.init(8);
    std::set<uint32_t> speeds;
    for (uint8_t i = 0; i < 8; ++i) {
        uint32_t base = sim.pilot_state(i).base_lap_ms;
        EXPECT_GE(base, 10000u);
        EXPECT_LE(base, 20000u);
        speeds.insert(base);
    }
    EXPECT_EQ(speeds.size(), 8u) << "All base speeds should be distinct";
}

TEST(SimulationInit, PilotsStartWithZeroLaps) {
    SimulationEngine sim;
    sim.init(6);
    for (uint8_t i = 0; i < 6; ++i) {
        EXPECT_EQ(sim.pilot_state(i).lap_count, 0);
        EXPECT_EQ(sim.pilot_state(i).total_time_ms, 0u);
        EXPECT_EQ(sim.pilot_state(i).last_lap_ms, 0u);
    }
}

// ─── Update / lap generation tests ──────────────────────────────

TEST(SimulationUpdate, NoChangeBeforeFirstLap) {
    SimulationEngine sim;
    sim.set_random_seed(42);
    sim.init(6);
    // At time 0, no laps should have completed yet
    sim.update(0);
    EXPECT_FALSE(sim.has_new_data());
}

TEST(SimulationUpdate, LapsCompleteAfterInterval) {
    SimulationEngine sim;
    sim.set_random_seed(42);
    sim.init(6);
    // Advance time well past the 2-4 second interval
    sim.update(5000);
    EXPECT_TRUE(sim.has_new_data());
}

TEST(SimulationUpdate, LapTimesInRange) {
    SimulationEngine sim;
    sim.set_random_seed(100);
    sim.init(6);

    // Run many updates to generate laps
    for (uint32_t t = 0; t < 60000; t += 500) {
        sim.update(t);
    }

    PilotEntry pilots[MAX_PILOTS];
    uint8_t count = sim.get_pilots(pilots, MAX_PILOTS);

    for (uint8_t i = 0; i < count; ++i) {
        if (pilots[i].lap_count > 0) {
            EXPECT_GE(pilots[i].last_lap_ms, 10000u)
                << "Pilot " << pilots[i].name << " last lap below range";
            EXPECT_LE(pilots[i].last_lap_ms, 20000u)
                << "Pilot " << pilots[i].name << " last lap above range";
            EXPECT_GE(pilots[i].best_lap_ms, 10000u)
                << "Pilot " << pilots[i].name << " best lap below range";
            EXPECT_LE(pilots[i].best_lap_ms, 20000u)
                << "Pilot " << pilots[i].name << " best lap above range";
        }
    }
}

TEST(SimulationUpdate, BestLapIsMinimum) {
    SimulationEngine sim;
    sim.set_random_seed(77);
    sim.init(5);

    // Run enough updates to generate multiple laps
    for (uint32_t t = 0; t < 30000; t += 500) {
        sim.update(t);
    }

    for (uint8_t i = 0; i < 5; ++i) {
        auto& state = sim.pilot_state(i);
        if (state.lap_count > 0) {
            EXPECT_LE(state.best_lap_ms, state.last_lap_ms)
                << "Best lap should be <= last lap for pilot " << state.name;
            EXPECT_GE(state.best_lap_ms, 10000u);
        }
    }
}

TEST(SimulationUpdate, TotalTimeIsConsistent) {
    SimulationEngine sim;
    sim.set_random_seed(55);
    sim.init(6);

    // Run simulation
    for (uint32_t t = 0; t < 20000; t += 500) {
        sim.update(t);
    }

    for (uint8_t i = 0; i < 6; ++i) {
        auto& state = sim.pilot_state(i);
        if (state.lap_count > 0) {
            // total_time should be at least lap_count * min_lap_time
            EXPECT_GE(state.total_time_ms, state.lap_count * 10000u);
            // total_time should be at most lap_count * max_lap_time
            EXPECT_LE(state.total_time_ms, state.lap_count * 20000u);
        }
    }
}

// ─── has_new_data / get_pilots flag tests ────────────────────────

TEST(SimulationFlag, GetPilotsResetsFlag) {
    SimulationEngine sim;
    sim.set_random_seed(42);
    sim.init(6);

    sim.update(5000);  // Should trigger laps
    EXPECT_TRUE(sim.has_new_data());

    PilotEntry pilots[MAX_PILOTS];
    sim.get_pilots(pilots, MAX_PILOTS);
    EXPECT_FALSE(sim.has_new_data());
}

TEST(SimulationFlag, NoNewDataWithoutChange) {
    SimulationEngine sim;
    sim.set_random_seed(42);
    sim.init(6);

    PilotEntry pilots[MAX_PILOTS];
    sim.get_pilots(pilots, MAX_PILOTS);
    EXPECT_FALSE(sim.has_new_data());

    // Update at time 0 — no laps complete
    sim.update(0);
    EXPECT_FALSE(sim.has_new_data());
}

// ─── get_pilots output tests ─────────────────────────────────────

TEST(SimulationGetPilots, ReturnsSortedByPosition) {
    SimulationEngine sim;
    sim.set_random_seed(42);
    sim.init(6);

    // Generate some laps
    for (uint32_t t = 0; t < 15000; t += 500) {
        sim.update(t);
    }

    PilotEntry pilots[MAX_PILOTS];
    uint8_t count = sim.get_pilots(pilots, MAX_PILOTS);

    EXPECT_EQ(count, 6);
    for (uint8_t i = 0; i < count; ++i) {
        EXPECT_EQ(pilots[i].position, i + 1);
    }
}

TEST(SimulationGetPilots, LeaderHasGapMinusOne) {
    SimulationEngine sim;
    sim.set_random_seed(42);
    sim.init(6);

    for (uint32_t t = 0; t < 15000; t += 500) {
        sim.update(t);
    }

    PilotEntry pilots[MAX_PILOTS];
    sim.get_pilots(pilots, MAX_PILOTS);

    EXPECT_EQ(pilots[0].gap_ms, -1);
}

TEST(SimulationGetPilots, RespectsMaxCount) {
    SimulationEngine sim;
    sim.init(8);

    PilotEntry pilots[4];
    uint8_t count = sim.get_pilots(pilots, 4);
    EXPECT_EQ(count, 4);
}
