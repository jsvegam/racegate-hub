#include "render.h"
#include <cstdio>
#include <cstring>

namespace render {

void init_render(RenderState& state) {
    std::memset(&state, 0, sizeof(RenderState));
    state.last_pilot_count = 0;
    state.header_drawn = false;
}

void format_time(uint32_t time_ms, char* buf, uint8_t buf_size) {
    if (!buf || buf_size == 0) return;
    uint32_t sec = time_ms / 1000;
    uint32_t ms  = time_ms % 1000;
    snprintf(buf, buf_size, "%d.%03d", (int)sec, (int)ms);
}

void format_gap(int32_t gap_ms, char* buf, uint8_t buf_size) {
    if (!buf || buf_size == 0) return;
    if (gap_ms == -1) {
        snprintf(buf, buf_size, "---");
    } else {
        uint32_t abs_gap = (uint32_t)gap_ms;
        uint32_t sec = abs_gap / 1000;
        uint32_t ms  = abs_gap % 1000;
        snprintf(buf, buf_size, "+%d.%03d", (int)sec, (int)ms);
    }
}

uint16_t get_position_color(uint8_t position) {
    return (position == 1) ? COLOR_GREEN : COLOR_LGREY;
}

uint16_t get_last_lap_color(uint32_t last_lap_ms, uint32_t worst_lap_ms) {
    if (last_lap_ms > 0 && worst_lap_ms > 0 && last_lap_ms == worst_lap_ms) {
        return COLOR_RED;
    }
    return COLOR_LGREY;
}

uint32_t find_worst_lap(const PilotEntry pilots[], uint8_t count) {
    uint32_t worst = 0;
    for (uint8_t i = 0; i < count; ++i) {
        if (pilots[i].lap_count > 0 && pilots[i].last_lap_ms > worst) {
            worst = pilots[i].last_lap_ms;
        }
    }
    return worst;
}

uint16_t get_gap_color(int32_t current_gap, int32_t prev_gap) {
    // Leader always green
    if (current_gap == -1) return COLOR_GREEN;
    // No previous data yet — neutral
    if (prev_gap == 0 || prev_gap == -1) return COLOR_LGREY;
    // Gap decreased (closer to leader) — green (improving)
    if (current_gap < prev_gap) return COLOR_GREEN;
    // Gap increased (falling behind) — red (worsening)
    if (current_gap > prev_gap) return COLOR_RED;
    // Same gap — neutral
    return COLOR_LGREY;
}

bool format_pilot_row(const PilotEntry& pilot, uint32_t worst_lap_ms,
                      int32_t prev_gap, CellCache new_cells[NUM_COLS]) {
    // Col 0: Position — green for leader, light grey for others
    snprintf(new_cells[0].text, 16, "%d", pilot.position);
    new_cells[0].fg_color = get_position_color(pilot.position);

    // Col 1: Name — amber for visibility
    snprintf(new_cells[1].text, 16, "%.12s", pilot.name);
    new_cells[1].fg_color = COLOR_AMBER;

    // Col 2: Last lap — red for worst, light grey for normal
    if (pilot.lap_count > 0) {
        format_time(pilot.last_lap_ms, new_cells[2].text, 16);
        new_cells[2].fg_color = get_last_lap_color(pilot.last_lap_ms, worst_lap_ms);
    } else {
        snprintf(new_cells[2].text, 16, "---.---");
        new_cells[2].fg_color = COLOR_ROW_ALT;
    }

    // Col 3: Best lap — cyan highlight
    if (pilot.best_lap_ms > 0) {
        format_time(pilot.best_lap_ms, new_cells[3].text, 16);
        new_cells[3].fg_color = COLOR_BEST_LAP;
    } else {
        snprintf(new_cells[3].text, 16, "---.---");
        new_cells[3].fg_color = COLOR_ROW_ALT;
    }

    // Col 4: Lap count — light grey
    snprintf(new_cells[4].text, 16, "%d", pilot.lap_count);
    new_cells[4].fg_color = COLOR_LGREY;

    // Col 5: Gap — colored by delta (green=improving, red=worsening, grey=same)
    format_gap(pilot.gap_ms, new_cells[5].text, 16);
    new_cells[5].fg_color = get_gap_color(pilot.gap_ms, prev_gap);

    return true;
}

// ─── LCD-dependent functions (only for target build) ─────────────

#ifndef NATIVE_BUILD

void draw_header(LGFX& lcd, RenderState& state) {
    const char* headers[] = { "POS", "PILOT", "LAST", "BEST", "LAPS", "GAP" };

    // Header background bar
    lcd.fillRect(0, 0, SCREEN_W, HEADER_H, COLOR_HEADER_BG);

    lcd.setTextSize(1);
    lcd.setFont(&lgfx::fonts::Font2);

    for (uint8_t c = 0; c < NUM_COLS; ++c) {
        lcd.setTextColor(COLOR_CYAN, COLOR_HEADER_BG);
        lcd.setCursor(COL_X[c] + 2, 8);
        lcd.print(headers[c]);
    }

    // Bright separator line under header
    lcd.drawFastHLine(0, HEADER_H - 1, SCREEN_W, COLOR_CYAN);
    state.header_drawn = true;
}

void render_dashboard(LGFX& lcd, RenderState& state,
                      const PilotEntry pilots[], uint8_t count) {
    if (!state.header_drawn) {
        draw_header(lcd, state);
    }

    uint32_t worst_lap = find_worst_lap(pilots, count);

    lcd.setTextSize(1);
    lcd.setFont(&lgfx::fonts::Font2);

    for (uint8_t row = 0; row < count; ++row) {
        CellCache new_cells[NUM_COLS];
        std::memset(new_cells, 0, sizeof(new_cells));
        format_pilot_row(pilots[row], worst_lap, state.prev_gap[row], new_cells);

        uint16_t y = HEADER_H + (row * ROW_H);

        // Alternating row background for readability
        uint16_t row_bg = (row % 2 == 0) ? COLOR_BLACK : COLOR_ROW_ALT;

        for (uint8_t col = 0; col < NUM_COLS; ++col) {
            if (std::strcmp(state.cells[row][col].text, new_cells[col].text) != 0 ||
                state.cells[row][col].fg_color != new_cells[col].fg_color) {

                lcd.fillRect(COL_X[col], y, COL_W[col], ROW_H, row_bg);
                lcd.setTextColor(new_cells[col].fg_color, row_bg);
                lcd.setCursor(COL_X[col] + 2, y + 10);
                lcd.print(new_cells[col].text);

                std::memcpy(&state.cells[row][col], &new_cells[col], sizeof(CellCache));
            }
        }

        // Store current gap for next comparison
        state.prev_gap[row] = pilots[row].gap_ms;
    }

    if (count < state.last_pilot_count) {
        for (uint8_t row = count; row < state.last_pilot_count; ++row) {
            uint16_t y = HEADER_H + (row * ROW_H);
            lcd.fillRect(0, y, SCREEN_W, ROW_H, COLOR_BLACK);
            for (uint8_t col = 0; col < NUM_COLS; ++col) {
                std::memset(&state.cells[row][col], 0, sizeof(CellCache));
            }
        }
    }

    state.last_pilot_count = count;
}

#endif // NATIVE_BUILD

} // namespace render
