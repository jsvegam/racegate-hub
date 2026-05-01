#pragma once

#include "data_provider.h"
#include <cstdio>
#include <cstring>

namespace render {

// Layout constants
static const uint16_t SCREEN_W = 480;
static const uint16_t SCREEN_H = 320;
static const uint8_t  HEADER_H = 30;
static const uint8_t  ROW_H    = 36;
static const uint8_t  NUM_COLS = 6;

// Column X positions and widths
static const uint16_t COL_X[]  = { 0,  30, 150, 240, 330, 390 };
static const uint16_t COL_W[]  = { 30, 120, 90,  90,  60,  90 };

// Color constants (TFT_eSPI 16-bit RGB565 format)
// These are defined as uint16_t so they work in both native and target builds
static const uint16_t COLOR_BLACK     = 0x0000;
static const uint16_t COLOR_WHITE     = 0xFFFF;
static const uint16_t COLOR_GREEN     = 0x07E0;
static const uint16_t COLOR_RED       = 0xF800;
static const uint16_t COLOR_DARKGREY  = 0x7BEF;

struct CellCache {
    char text[16];
    uint16_t fg_color;
};

struct RenderState {
    CellCache cells[MAX_PILOTS][NUM_COLS];
    uint8_t   last_pilot_count;
    bool      header_drawn;
};

// Initializes render state (clears cache)
void init_render(RenderState& state);

// Formatting helpers — available in native builds for testing
void format_time(uint32_t time_ms, char* buf, uint8_t buf_size);
void format_gap(int32_t gap_ms, char* buf, uint8_t buf_size);

// Color logic — available in native builds for testing
// Returns the foreground color for the position column
uint16_t get_position_color(uint8_t position);

// Returns the foreground color for the last-lap column
// worst_lap_ms is the highest last_lap_ms among all active pilots
uint16_t get_last_lap_color(uint32_t last_lap_ms, uint32_t worst_lap_ms);

// Finds the worst (highest) last_lap_ms among pilots with lap_count > 0
uint32_t find_worst_lap(const PilotEntry pilots[], uint8_t count);

// Formats a pilot row into the cells array, returns true if any cell changed
bool format_pilot_row(const PilotEntry& pilot, uint32_t worst_lap_ms,
                      CellCache new_cells[NUM_COLS]);

#ifndef NATIVE_BUILD
} // close namespace for native-only section

#include "display.h"

namespace render {
    void draw_header(LGFX& lcd, RenderState& state);
    void render_dashboard(LGFX& lcd, RenderState& state,
                          const PilotEntry pilots[], uint8_t count);
#endif

} // namespace render
