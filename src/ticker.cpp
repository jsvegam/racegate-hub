#include "ticker.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace ticker {

// Race commentary templates
static const char* BURNING_MSGS[] = {
    "%s is ON FIRE!",
    "%s is BURNING IT!",
    "%s UNSTOPPABLE!",
    "%s FLYING LOW!",
};
static const uint8_t NUM_BURNING = 4;

static const char* CHASE_MSGS[] = {
    "%s hunting down %s!",
    "%s closing on %s!",
    "%s chasing %s!",
    "%s on %s's tail!",
};
static const uint8_t NUM_CHASE = 4;

static const char* OVERTAKE_MSGS[] = {
    "%s OVERTAKES %s!",
    "%s passes %s!",
    "%s takes P%d!",
};
static const uint8_t NUM_OVERTAKE = 3;

static const char* LEADER_MSGS[] = {
    "%s leads the pack!",
    "%s in command!",
    "%s P1 and pulling away!",
};
static const uint8_t NUM_LEADER = 3;

static uint32_t simple_rand_seed = 12345;
static uint32_t simple_rand() {
    simple_rand_seed = simple_rand_seed * 1103515245u + 12345u;
    return (simple_rand_seed >> 16) & 0x7FFF;
}

void init_ticker(TickerState& state) {
    std::memset(&state, 0, sizeof(TickerState));
    state.phase = PHASE_IDLE;
    state.first_update = true;
    state.display_duration_ms = 3000;
    state.cooldown_ms = 8000;  // Minimum 8 seconds between messages
    state.last_message_ms = 0;
    state.area_dirty = false;
}

bool generate_message(TickerState& state, const PilotEntry pilots[], uint8_t count) {
    if (count < 2) return false;
    if (state.phase != PHASE_IDLE) return false;

    // On first update, just store state
    if (state.first_update) {
        for (uint8_t i = 0; i < count; ++i) {
            state.prev_gaps[i] = pilots[i].gap_ms;
            state.prev_positions[i] = pilots[i].position;
        }
        state.first_update = false;
        return false;
    }

    // Cooldown check — don't spam messages
    uint32_t now = 0;
#ifndef NATIVE_BUILD
    now = millis();
#endif
    if (state.last_message_ms > 0 && (now - state.last_message_ms) < state.cooldown_ms) {
        // Still update prev state even if we don't show a message
        for (uint8_t i = 0; i < count; ++i) {
            state.prev_gaps[i] = pilots[i].gap_ms;
            state.prev_positions[i] = pilots[i].position;
        }
        return false;
    }

    // Check for events and generate message
    char msg[64] = {0};
    uint16_t color = 0xFFFF;
    bool found = false;

    // Priority 1: Overtake detected (position changed)
    for (uint8_t i = 0; i < count && !found; ++i) {
        if (pilots[i].position < state.prev_positions[i] && pilots[i].lap_count > 0) {
            // This pilot moved up
            // Find who they overtook
            for (uint8_t j = 0; j < count; ++j) {
                if (j != i && pilots[j].position == pilots[i].position + 1 &&
                    state.prev_positions[j] == pilots[i].position) {
                    uint8_t idx = simple_rand() % NUM_OVERTAKE;
                    if (idx < 2) {
                        snprintf(msg, sizeof(msg), OVERTAKE_MSGS[idx],
                                 pilots[i].name, pilots[j].name);
                    } else {
                        snprintf(msg, sizeof(msg), OVERTAKE_MSGS[idx],
                                 pilots[i].name, pilots[i].position);
                    }
                    color = 0x07E0; // Green
                    found = true;
                    break;
                }
            }
            if (!found) {
                snprintf(msg, sizeof(msg), OVERTAKE_MSGS[2],
                         pilots[i].name, pilots[i].position);
                color = 0x07E0;
                found = true;
            }
        }
    }

    // Priority 2: Gap closing fast (pilot getting closer to the one ahead)
    if (!found) {
        for (uint8_t i = 1; i < count; ++i) {
            if (pilots[i].gap_ms > 0 && state.prev_gaps[i] > 0 &&
                pilots[i].gap_ms < state.prev_gaps[i] - 2000 &&
                pilots[i].lap_count > 2) {
                // Find pilot ahead
                uint8_t ahead_idx = 0;
                for (uint8_t j = 0; j < count; ++j) {
                    if (pilots[j].position == pilots[i].position - 1) {
                        ahead_idx = j;
                        break;
                    }
                }
                uint8_t idx = simple_rand() % NUM_CHASE;
                snprintf(msg, sizeof(msg), CHASE_MSGS[idx],
                         pilots[i].name, pilots[ahead_idx].name);
                color = 0xFE60; // Amber
                found = true;
                break;
            }
        }
    }

    // Priority 3: Leader pulling away
    if (!found && pilots[0].lap_count > 3) {
        if (count > 1 && pilots[1].gap_ms > 0 &&
            state.prev_gaps[1] > 0 &&
            pilots[1].gap_ms > state.prev_gaps[1] + 1500) {
            uint8_t idx = simple_rand() % NUM_LEADER;
            snprintf(msg, sizeof(msg), LEADER_MSGS[idx], pilots[0].name);
            color = 0x07E0; // Green
            found = true;
        }
    }

    // Priority 4: Best lap set (rare — only 1 in 15 chance)
    if (!found && (simple_rand() % 15 == 0)) {
        // Find pilot with best overall lap
        uint8_t best_idx = 0;
        uint32_t best_time = UINT32_MAX;
        for (uint8_t i = 0; i < count; ++i) {
            if (pilots[i].best_lap_ms > 0 && pilots[i].best_lap_ms < best_time) {
                best_time = pilots[i].best_lap_ms;
                best_idx = i;
            }
        }
        if (best_time < UINT32_MAX) {
            uint8_t idx = simple_rand() % NUM_BURNING;
            snprintf(msg, sizeof(msg), BURNING_MSGS[idx], pilots[best_idx].name);
            color = 0xF800; // Red hot
            found = true;
        }
    }

    // Update previous state
    for (uint8_t i = 0; i < count; ++i) {
        state.prev_gaps[i] = pilots[i].gap_ms;
        state.prev_positions[i] = pilots[i].position;
    }

    if (found && msg[0] != '\0') {
        std::strncpy(state.message, msg, sizeof(state.message) - 1);
        state.msg_color = color;
        state.phase = PHASE_DISPLAY;
        state.phase_start_ms = 0;
        state.last_message_ms = now;
        return true;
    }

    return false;
}

void update_ticker(TickerState& state, uint32_t now_ms) {
    if (state.phase == PHASE_DISPLAY) {
        if (state.phase_start_ms == 0) {
            state.phase_start_ms = now_ms;
        }
        if (now_ms - state.phase_start_ms > state.display_duration_ms) {
            // Time to explode!
            state.phase = PHASE_EXPLODE;
            state.phase_start_ms = now_ms;
        }
    } else if (state.phase == PHASE_EXPLODE) {
        // Update particles
        bool any_alive = false;
        for (uint8_t i = 0; i < state.particle_count; ++i) {
            Particle& p = state.particles[i];
            if (!p.active) continue;
            p.x += p.vx;
            p.y += p.vy;
            p.vy += 1; // Gravity
            if (p.life > 0) {
                p.life--;
            }
            if (p.life == 0 || p.y > 320 || p.x < 0 || p.x > 480) {
                p.active = false;
            } else {
                any_alive = true;
            }
        }
        if (!any_alive) {
            state.phase = PHASE_IDLE;
            state.message[0] = '\0';
            state.particle_count = 0;
        }
    }
}

#ifndef NATIVE_BUILD

// Create particles from the text area
static void spawn_particles(LGFX& lcd, TickerState& state) {
    state.particle_count = 0;
    uint16_t text_len = std::strlen(state.message);
    if (text_len == 0) return;

    // Calculate text bounds
    uint16_t text_w = text_len * 7; // Approximate char width for Font2
    uint16_t text_x = (TICKER_W - text_w) / 2;
    uint16_t text_y = TICKER_Y + 10;

    // Sample pixels from the text area and create particles
    for (uint8_t i = 0; i < MAX_PARTICLES && i < text_len * 4; ++i) {
        Particle& p = state.particles[i];
        // Distribute particles across the text
        p.x = text_x + (simple_rand() % text_w);
        p.y = text_y + (simple_rand() % 16);
        // Random velocity — explode outward
        p.vx = (int16_t)(simple_rand() % 7) - 3;
        p.vy = -(int16_t)(simple_rand() % 5) - 1;
        p.color = state.msg_color;
        p.life = 8 + (simple_rand() % 12); // 8-20 frames
        p.active = true;
        state.particle_count++;
    }
}

void render_ticker(LGFX& lcd, TickerState& state, uint32_t now_ms) {
    update_ticker(state, now_ms);

    if (state.phase == PHASE_DISPLAY) {
        if (state.phase_start_ms == 0 || !state.area_dirty) {
            // First frame or just entered display phase — draw text
            lcd.fillRect(0, TICKER_Y, TICKER_W, TICKER_H, 0x0000);
            lcd.setFont(&lgfx::fonts::Font2);
            lcd.setTextSize(1);
            lcd.setTextColor(state.msg_color, 0x0000);
            lcd.setTextDatum(middle_center);
            lcd.drawString(state.message, TICKER_W / 2, TICKER_Y + TICKER_H / 2);
            lcd.setTextDatum(top_left);
            state.area_dirty = true;
        }
        // Don't redraw every frame — text is static

    } else if (state.phase == PHASE_EXPLODE) {
        if (state.particle_count == 0) {
            spawn_particles(lcd, state);
            lcd.fillRect(0, TICKER_Y, TICKER_W, TICKER_H, 0x0000);
        }

        // Only redraw particle area
        lcd.fillRect(0, TICKER_Y - 5, TICKER_W, TICKER_H + 20, 0x0000);
        for (uint8_t i = 0; i < state.particle_count; ++i) {
            Particle& p = state.particles[i];
            if (!p.active) continue;
            if (p.x >= 0 && p.x < 480 && p.y >= TICKER_Y - 5 && p.y < 320) {
                uint16_t c = p.color;
                if (p.life < 6) {
                    uint8_t r = ((c >> 11) & 0x1F) * p.life / 6;
                    uint8_t g = ((c >> 5) & 0x3F) * p.life / 6;
                    uint8_t b = (c & 0x1F) * p.life / 6;
                    c = (r << 11) | (g << 5) | b;
                }
                lcd.fillRect(p.x, p.y, 2, 2, c);
            }
        }

    } else if (state.phase == PHASE_IDLE && state.area_dirty) {
        // Clean up once when returning to idle
        lcd.fillRect(0, TICKER_Y - 5, TICKER_W, TICKER_H + 20, 0x0000);
        state.area_dirty = false;
    }
}

#endif // NATIVE_BUILD

} // namespace ticker
