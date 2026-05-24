#pragma once

#include "data_provider.h"
#include <cstdint>

#ifndef NATIVE_BUILD
#include <WiFi.h>
#include <WebServer.h>
#endif

// Configuration for FPVGate WiFi connection
struct FPVGateConfig {
    const char* ssid;           // WiFi SSID of FPVGate AP (e.g., "FPVGate_E110")
    const char* password;       // WiFi password (default: "fpvgate1")
    uint16_t server_port;       // HTTP server port to listen on (default: 80)
};

// Internal state for each pilot (identified by timer node IP)
struct PilotNode {
    char     name[NAME_MAX_LEN + 1];  // Pilot name (from config or IP-based)
    char     ip[16];                   // IP address of the timer node
    uint32_t last_lap_ms;              // Last lap time in ms
    uint32_t best_lap_ms;              // Best lap time in ms
    uint16_t lap_count;                // Total laps completed
    uint32_t total_time_ms;            // Accumulated total time
    uint32_t last_lap_timestamp_ms;    // millis() when last lap was registered
    bool     active;                   // Whether this pilot is in the current race
};

class FPVGateProvider : public DataProvider {
public:
    FPVGateProvider();

    // Initialize WiFi connection and HTTP server
    bool init(const FPVGateConfig& config);

    // DataProvider interface
    void update() override;
    bool has_new_data() const override;
    uint8_t get_pilots(PilotEntry pilots[], uint8_t max_count) override;

    // Status
    bool is_connected() const;
    uint8_t get_pilot_count() const { return num_pilots_; }

private:
    PilotNode pilots_[MAX_PILOTS];
    uint8_t   num_pilots_;
    bool      data_changed_;
    bool      wifi_connected_;
    bool      race_active_;
    uint32_t  race_start_ms_;
    uint32_t  last_wifi_check_ms_;

#ifndef NATIVE_BUILD
    WebServer* server_;

    // HTTP handlers
    void handle_lap();
    void handle_race_start();
    void handle_race_stop();
    void handle_not_found();
#endif

    // Find or create pilot by IP
    int8_t find_pilot_by_ip(const char* ip);
    int8_t register_pilot(const char* ip);

    // Recalculate positions based on laps and total time
    void recalculate_positions(PilotEntry output[], uint8_t count);
};
