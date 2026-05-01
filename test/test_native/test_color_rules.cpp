#include <gtest/gtest.h>
#include "render.h"
#include <cstring>

TEST(ColorRules, LeaderPositionIsGreen) {
    EXPECT_EQ(render::get_position_color(1), render::COLOR_GREEN);
}

TEST(ColorRules, NonLeaderPositionIsWhite) {
    for (uint8_t pos = 2; pos <= 8; ++pos) {
        EXPECT_EQ(render::get_position_color(pos), render::COLOR_WHITE)
            << "Position " << (int)pos << " should be white";
    }
}

TEST(ColorRules, WorstLapIsRed) {
    EXPECT_EQ(render::get_last_lap_color(18000, 18000), render::COLOR_RED);
}

TEST(ColorRules, NonWorstLapIsWhite) {
    EXPECT_EQ(render::get_last_lap_color(12000, 18000), render::COLOR_WHITE);
}

TEST(ColorRules, ZeroLapTimeIsWhite) {
    EXPECT_EQ(render::get_last_lap_color(0, 18000), render::COLOR_WHITE);
}

TEST(ColorRules, FindWorstLapBasic) {
    PilotEntry pilots[3] = {};
    std::strcpy(pilots[0].name, "A");
    pilots[0].lap_count = 2; pilots[0].last_lap_ms = 12000;
    std::strcpy(pilots[1].name, "B");
    pilots[1].lap_count = 3; pilots[1].last_lap_ms = 18000;
    std::strcpy(pilots[2].name, "C");
    pilots[2].lap_count = 1; pilots[2].last_lap_ms = 15000;

    EXPECT_EQ(render::find_worst_lap(pilots, 3), 18000u);
}

TEST(ColorRules, FindWorstLapIgnoresZeroLaps) {
    PilotEntry pilots[2] = {};
    std::strcpy(pilots[0].name, "A");
    pilots[0].lap_count = 0; pilots[0].last_lap_ms = 99000;
    std::strcpy(pilots[1].name, "B");
    pilots[1].lap_count = 1; pilots[1].last_lap_ms = 15000;

    EXPECT_EQ(render::find_worst_lap(pilots, 2), 15000u);
}

TEST(ColorRules, FindWorstLapEmpty) {
    EXPECT_EQ(render::find_worst_lap(nullptr, 0), 0u);
}
