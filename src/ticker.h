#pragma once

#include "data_provider.h"
#include <cstdint>

#ifndef NATIVE_BUILD
#include "display.h"
#endif

namespace ticker {

// Ticker area: bottom of screen, below the pilot rows
static const uint16_t TICKER_Y = 282;  // After 8 rows: 30 + (8*36) = 318, but we use 282 for 7 rows margin
static const uint16_t TICKER_H = 38;   // Height of ticker area
static const uint16_t TICKER_W = 480;

// Particle for disintegration effect
struct Particle {
    int16_t x, y;       // Current position
    int16_t vx, vy;     // Velocity
    uint16_t color;     // Pixel color
    uint8_t life;       // Remaining life (frames)
    bool active;
};

static const uint8_t MAX_PARTICLES = 120;

// Ticker state
enum TickerPhase {
    PHASE_IDLE,         // Waiting for next message
    PHASE_DISPLAY,      // Showing text
    PHASE_EXPLODE,      // Particles flying
};

struct TickerState {
    TickerPhase phase;
    char message[64];
    uint16_t msg_color;
    uint32_t phase_start_ms;
    uint32_t display_duration_ms;
    uint32_t last_message_ms;       // Cooldown: when last message was shown
    uint32_t cooldown_ms;           // Minimum time between messages
    Particle particles[MAX_PARTICLES];
    uint8_t particle_count;
    int32_t prev_gaps[MAX_PILOTS];      // Track gap changes for commentary
    uint8_t prev_positions[MAX_PILOTS]; // Track position changes
    bool first_update;
    bool area_dirty;                // True if ticker area needs clearing
};

void init_ticker(TickerState& state);

// Generate a contextual message based on race events
// Returns true if a new message was generated
bool generate_message(TickerState& state, const PilotEntry pilots[], uint8_t count);

// Update ticker animation (call every frame)
void update_ticker(TickerState& state, uint32_t now_ms);

#ifndef NATIVE_BUILD
// Render ticker area on LCD
void render_ticker(LGFX& lcd, TickerState& state, uint32_t now_ms);
#endif

} // namespace ticker
