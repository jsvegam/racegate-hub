#include <gtest/gtest.h>
#include "render.h"
#include <cstring>

static PilotEntry make_pilot(const char* name, uint8_t pos, uint32_t last,
                              uint32_t best, uint16_t laps, int32_t gap, uint32_t total) {
    PilotEntry p{};
    std::strncpy(p.name, name, NAME_MAX_LEN);
    p.position = pos;
    p.last_lap_ms = last;
    p.best_lap_ms = best;
    p.lap_count = laps;
    p.gap_ms = gap;
    p.total_time_ms = total;
    return p;
}

TEST(FormatPilotRow, LeaderRowHasGreenPosition) {
    PilotEntry pilot = make_pilot("RAZOR", 1, 12345, 11000, 5, -1, 60000);
    render::CellCache cells[render::NUM_COLS];
    std::memset(cells, 0, sizeof(cells));

    render::format_pilot_row(pilot, 18000, 0, cells);

    EXPECT_STREQ(cells[0].text, "1");
    EXPECT_EQ(cells[0].fg_color, render::COLOR_GREEN);
}

TEST(FormatPilotRow, NonLeaderRowHasWhitePosition) {
    PilotEntry pilot = make_pilot("VIPER", 3, 14000, 12000, 4, 2000, 55000);
    render::CellCache cells[render::NUM_COLS];
    std::memset(cells, 0, sizeof(cells));

    render::format_pilot_row(pilot, 18000, 0, cells);

    EXPECT_STREQ(cells[0].text, "3");
    EXPECT_EQ(cells[0].fg_color, render::COLOR_LGREY);
}

TEST(FormatPilotRow, WorstLapIsRed) {
    PilotEntry pilot = make_pilot("GHOST", 2, 18000, 12000, 3, 1000, 50000);
    render::CellCache cells[render::NUM_COLS];
    std::memset(cells, 0, sizeof(cells));

    render::format_pilot_row(pilot, 18000, 0, cells);

    EXPECT_EQ(cells[2].fg_color, render::COLOR_RED);
}

TEST(FormatPilotRow, NormalLapIsWhite) {
    PilotEntry pilot = make_pilot("STORM", 2, 14000, 12000, 3, 1000, 50000);
    render::CellCache cells[render::NUM_COLS];
    std::memset(cells, 0, sizeof(cells));

    render::format_pilot_row(pilot, 18000, 0, cells);

    EXPECT_EQ(cells[2].fg_color, render::COLOR_LGREY);
}

TEST(FormatPilotRow, ZeroLapsShowsDashes) {
    PilotEntry pilot = make_pilot("BLITZ", 5, 0, 0, 0, 5000, 0);
    render::CellCache cells[render::NUM_COLS];
    std::memset(cells, 0, sizeof(cells));

    render::format_pilot_row(pilot, 18000, 0, cells);

    EXPECT_STREQ(cells[2].text, "---.---");
    EXPECT_STREQ(cells[3].text, "---.---");
    EXPECT_STREQ(cells[4].text, "0");
}

TEST(FormatPilotRow, GapLeaderShowsDashes) {
    PilotEntry pilot = make_pilot("RAZOR", 1, 12000, 11000, 5, -1, 60000);
    render::CellCache cells[render::NUM_COLS];
    std::memset(cells, 0, sizeof(cells));

    render::format_pilot_row(pilot, 18000, 0, cells);

    EXPECT_STREQ(cells[5].text, "---");
}

TEST(FormatPilotRow, GapNonLeaderShowsPlus) {
    PilotEntry pilot = make_pilot("VIPER", 2, 14000, 12000, 4, 3456, 63456);
    render::CellCache cells[render::NUM_COLS];
    std::memset(cells, 0, sizeof(cells));

    render::format_pilot_row(pilot, 18000, 0, cells);

    EXPECT_STREQ(cells[5].text, "+3.456");
}

TEST(PartialUpdate, CacheDetectsChanges) {
    render::RenderState state;
    render::init_render(state);

    // Simulate first render — all cells empty, so everything should differ
    PilotEntry pilot = make_pilot("RAZOR", 1, 12345, 11000, 5, -1, 60000);
    render::CellCache new_cells[render::NUM_COLS];
    std::memset(new_cells, 0, sizeof(new_cells));
    render::format_pilot_row(pilot, 18000, 0, new_cells);

    // All cells should differ from initial empty state
    for (uint8_t c = 0; c < render::NUM_COLS; ++c) {
        bool text_changed = std::strcmp(state.cells[0][c].text, new_cells[c].text) != 0;
        bool color_changed = state.cells[0][c].fg_color != new_cells[c].fg_color;
        EXPECT_TRUE(text_changed || color_changed)
            << "Cell " << (int)c << " should have changed on first render";
    }

    // Copy to cache (simulating what render_dashboard does)
    std::memcpy(state.cells[0], new_cells, sizeof(new_cells));

    // Same data again — nothing should change
    render::CellCache same_cells[render::NUM_COLS];
    std::memset(same_cells, 0, sizeof(same_cells));
    render::format_pilot_row(pilot, 18000, 0, same_cells);

    for (uint8_t c = 0; c < render::NUM_COLS; ++c) {
        bool text_same = std::strcmp(state.cells[0][c].text, same_cells[c].text) == 0;
        bool color_same = state.cells[0][c].fg_color == same_cells[c].fg_color;
        EXPECT_TRUE(text_same && color_same)
            << "Cell " << (int)c << " should NOT have changed on same data";
    }
}
