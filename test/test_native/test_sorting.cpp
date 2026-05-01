#include <gtest/gtest.h>
#include "race_logic.h"
#include <cstring>

// Helper to create a PilotEntry with specific fields
static PilotEntry make_pilot(const char* name, uint16_t laps, uint32_t total_ms) {
    PilotEntry p{};
    std::strncpy(p.name, name, NAME_MAX_LEN);
    p.name[NAME_MAX_LEN] = '\0';
    p.lap_count = laps;
    p.total_time_ms = total_ms;
    p.position = 0;
    p.gap_ms = 0;
    p.last_lap_ms = 0;
    p.best_lap_ms = 0;
    return p;
}

// ─── sort_pilots tests ───────────────────────────────────────────

TEST(SortPilots, SortsByHigherLapCountFirst) {
    PilotEntry pilots[3] = {
        make_pilot("Alice", 3, 45000),
        make_pilot("Bob",   5, 60000),
        make_pilot("Carol", 4, 50000),
    };

    race_logic::sort_pilots(pilots, 3);

    // Bob (5 laps) → Carol (4 laps) → Alice (3 laps)
    EXPECT_STREQ(pilots[0].name, "Bob");
    EXPECT_STREQ(pilots[1].name, "Carol");
    EXPECT_STREQ(pilots[2].name, "Alice");
}

TEST(SortPilots, TiebreaksByLowerTotalTime) {
    PilotEntry pilots[3] = {
        make_pilot("Alice", 5, 55000),
        make_pilot("Bob",   5, 50000),
        make_pilot("Carol", 5, 52000),
    };

    race_logic::sort_pilots(pilots, 3);

    // Same laps → sorted by ascending total_time_ms
    EXPECT_STREQ(pilots[0].name, "Bob");    // 50000
    EXPECT_STREQ(pilots[1].name, "Carol");  // 52000
    EXPECT_STREQ(pilots[2].name, "Alice");  // 55000
}

TEST(SortPilots, AssignsPositions1ToN) {
    PilotEntry pilots[4] = {
        make_pilot("D", 2, 30000),
        make_pilot("A", 5, 60000),
        make_pilot("B", 3, 40000),
        make_pilot("C", 4, 50000),
    };

    race_logic::sort_pilots(pilots, 4);

    EXPECT_EQ(pilots[0].position, 1);
    EXPECT_EQ(pilots[1].position, 2);
    EXPECT_EQ(pilots[2].position, 3);
    EXPECT_EQ(pilots[3].position, 4);
}

TEST(SortPilots, MixedLapsAndTimeTiebreak) {
    // Two pilots with 5 laps (tiebreak by time), two with 3 laps (tiebreak by time)
    PilotEntry pilots[4] = {
        make_pilot("A", 3, 40000),
        make_pilot("B", 5, 55000),
        make_pilot("C", 5, 50000),
        make_pilot("D", 3, 35000),
    };

    race_logic::sort_pilots(pilots, 4);

    EXPECT_STREQ(pilots[0].name, "C");  // 5 laps, 50000
    EXPECT_STREQ(pilots[1].name, "B");  // 5 laps, 55000
    EXPECT_STREQ(pilots[2].name, "D");  // 3 laps, 35000
    EXPECT_STREQ(pilots[3].name, "A");  // 3 laps, 40000
}

TEST(SortPilots, SinglePilot) {
    PilotEntry pilots[1] = {
        make_pilot("Solo", 3, 30000),
    };

    race_logic::sort_pilots(pilots, 1);

    EXPECT_STREQ(pilots[0].name, "Solo");
    EXPECT_EQ(pilots[0].position, 1);
}

TEST(SortPilots, AllSameLapsAndTime) {
    PilotEntry pilots[3] = {
        make_pilot("A", 4, 40000),
        make_pilot("B", 4, 40000),
        make_pilot("C", 4, 40000),
    };

    race_logic::sort_pilots(pilots, 3);

    // All equal — positions should still be assigned 1, 2, 3
    EXPECT_EQ(pilots[0].position, 1);
    EXPECT_EQ(pilots[1].position, 2);
    EXPECT_EQ(pilots[2].position, 3);
}

TEST(SortPilots, EmptyArray) {
    race_logic::sort_pilots(nullptr, 0);
    // Should not crash
}

// ─── calculate_gaps tests ────────────────────────────────────────

TEST(CalculateGaps, LeaderGapIsMinusOne) {
    PilotEntry pilots[3] = {
        make_pilot("Leader", 5, 50000),
        make_pilot("Second", 5, 52000),
        make_pilot("Third",  4, 48000),
    };
    // Assume already sorted (leader at index 0)
    pilots[0].position = 1;
    pilots[1].position = 2;
    pilots[2].position = 3;

    race_logic::calculate_gaps(pilots, 3);

    EXPECT_EQ(pilots[0].gap_ms, -1);
}

TEST(CalculateGaps, OtherPilotsGapIsDifference) {
    PilotEntry pilots[3] = {
        make_pilot("Leader", 5, 50000),
        make_pilot("Second", 5, 52000),
        make_pilot("Third",  4, 55000),
    };
    pilots[0].position = 1;
    pilots[1].position = 2;
    pilots[2].position = 3;

    race_logic::calculate_gaps(pilots, 3);

    EXPECT_EQ(pilots[0].gap_ms, -1);
    EXPECT_EQ(pilots[1].gap_ms, 2000);   // Same laps: 52000 - 50000
    // Third has fewer laps: gap = 1 lap * (50000/5) = 10000
    EXPECT_EQ(pilots[2].gap_ms, 10000);
}

TEST(CalculateGaps, SinglePilotIsLeader) {
    PilotEntry pilots[1] = {
        make_pilot("Solo", 3, 30000),
    };
    pilots[0].position = 1;

    race_logic::calculate_gaps(pilots, 1);

    EXPECT_EQ(pilots[0].gap_ms, -1);
}

TEST(CalculateGaps, EmptyArray) {
    race_logic::calculate_gaps(nullptr, 0);
    // Should not crash
}

// ─── Integration: sort then calculate gaps ───────────────────────

TEST(SortAndGap, FullWorkflowSameLaps) {
    // All pilots have the same lap count — gaps are straightforward
    PilotEntry pilots[3] = {
        make_pilot("C", 5, 55000),
        make_pilot("A", 5, 50000),
        make_pilot("B", 5, 52000),
    };

    race_logic::sort_pilots(pilots, 3);
    race_logic::calculate_gaps(pilots, 3);

    // After sort: A(5,50000), B(5,52000), C(5,55000)
    EXPECT_STREQ(pilots[0].name, "A");
    EXPECT_EQ(pilots[0].position, 1);
    EXPECT_EQ(pilots[0].gap_ms, -1);

    EXPECT_STREQ(pilots[1].name, "B");
    EXPECT_EQ(pilots[1].position, 2);
    EXPECT_EQ(pilots[1].gap_ms, 2000);   // 52000 - 50000

    EXPECT_STREQ(pilots[2].name, "C");
    EXPECT_EQ(pilots[2].position, 3);
    EXPECT_EQ(pilots[2].gap_ms, 5000);   // 55000 - 50000
}

TEST(SortAndGap, FullWorkflowDifferentLaps) {
    // Pilots with different lap counts — leader has most laps
    PilotEntry pilots[3] = {
        make_pilot("C", 3, 35000),
        make_pilot("A", 5, 60000),
        make_pilot("B", 4, 48000),
    };

    race_logic::sort_pilots(pilots, 3);
    race_logic::calculate_gaps(pilots, 3);

    // After sort: A(5,60000), B(4,48000), C(3,35000)
    EXPECT_STREQ(pilots[0].name, "A");
    EXPECT_EQ(pilots[0].position, 1);
    EXPECT_EQ(pilots[0].gap_ms, -1);

    // B has 1 fewer lap: gap = 1 * (60000/5) = 12000
    EXPECT_STREQ(pilots[1].name, "B");
    EXPECT_EQ(pilots[1].position, 2);
    EXPECT_EQ(pilots[1].gap_ms, 12000);

    // C has 2 fewer laps: gap = 2 * (60000/5) = 24000
    EXPECT_STREQ(pilots[2].name, "C");
    EXPECT_EQ(pilots[2].position, 3);
    EXPECT_EQ(pilots[2].gap_ms, 24000);
}
