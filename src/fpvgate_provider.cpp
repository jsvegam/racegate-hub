#ifndef NATIVE_BUILD

#include "fpvgate_provider.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <algorithm>
#include <cstring>

FPVGateProvider::FPVGateProvider()
    : num_pilots_(0)
    , data_changed_(false)
    , wifi_connected_(false)
    , race_active_(false)
    , race_start_ms_(0)
    , last_wifi_check_ms_(0)
    , server_(nullptr)
    , lap_history_count_(0)
    , best_overall_ms_(0)
{
    memset(pilots_, 0, sizeof(pilots_));
    memset(lap_history_, 0, sizeof(lap_history_));
}

bool FPVGateProvider::init() {
    // WiFi is already connected via WiFiManager before this is called
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[FPVGate] WiFi not connected! Cannot start server.");
        wifi_connected_ = false;
        return false;
    }

    wifi_connected_ = true;

    // Start HTTP server to receive webhooks
    server_ = new WebServer(80);

    server_->on("/Lap", HTTP_POST, [this]() { handle_lap(); });
    server_->on("/RaceStart", HTTP_POST, [this]() { handle_race_start(); });
    server_->on("/RaceStop", HTTP_POST, [this]() { handle_race_stop(); });
    server_->onNotFound([this]() { handle_not_found(); });

    server_->begin();
    Serial.printf("[FPVGate] HTTP server started on port 80\n");
    Serial.printf("[FPVGate] Display IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.println("[FPVGate] Listening for webhooks: /Lap, /RaceStart, /RaceStop");

    return true;
}

void FPVGateProvider::update() {
    if (!wifi_connected_) return;

    // Handle incoming HTTP requests
    if (server_) {
        server_->handleClient();
    }

    // Periodically check WiFi connection
    uint32_t now = millis();
    if (now - last_wifi_check_ms_ > 5000) {
        last_wifi_check_ms_ = now;
        if (WiFi.status() != WL_CONNECTED) {
            wifi_connected_ = false;
            Serial.println("[FPVGate] WiFi disconnected! Attempting reconnect...");
            WiFi.reconnect();
        } else {
            wifi_connected_ = true;
        }
    }
}

bool FPVGateProvider::has_new_data() const {
    return data_changed_;
}

uint8_t FPVGateProvider::get_pilots(PilotEntry pilots[], uint8_t max_count) {
    uint8_t count = 0;

    for (uint8_t i = 0; i < num_pilots_ && count < max_count; i++) {
        if (!pilots_[i].active) continue;

        pilots[count].position = 0;
        strncpy(pilots[count].name, pilots_[i].name, NAME_MAX_LEN);
        pilots[count].name[NAME_MAX_LEN] = '\0';
        pilots[count].last_lap_ms = pilots_[i].last_lap_ms;
        pilots[count].best_lap_ms = pilots_[i].best_lap_ms;
        pilots[count].lap_count = pilots_[i].lap_count;
        pilots[count].total_time_ms = pilots_[i].total_time_ms;
        pilots[count].gap_ms = 0;
        count++;
    }

    if (count > 0) {
        recalculate_positions(pilots, count);
    }

    data_changed_ = false;
    return count;
}

bool FPVGateProvider::is_connected() const {
    return wifi_connected_;
}

// ============================================================
// HTTP Webhook Handlers
// ============================================================

void FPVGateProvider::handle_lap() {
    String client_ip = server_->client().remoteIP().toString();
    String body = server_->arg("plain");
    Serial.printf("[FPVGate] /Lap from %s\n", client_ip.c_str());
    Serial.printf("[FPVGate] Body: %s\n", body.c_str());
    
    // Log all arguments
    for (int i = 0; i < server_->args(); i++) {
        Serial.printf("[FPVGate] Arg[%s] = %s\n", 
                      server_->argName(i).c_str(), 
                      server_->arg(i).c_str());
    }
    
    // Log all headers
    for (int i = 0; i < server_->headers(); i++) {
        Serial.printf("[FPVGate] Header[%s] = %s\n",
                      server_->headerName(i).c_str(),
                      server_->header(i).c_str());
    }

    int8_t idx = find_pilot_by_ip(client_ip.c_str());
    if (idx < 0) {
        idx = register_pilot(client_ip.c_str());
        if (idx < 0) {
            server_->send(503, "text/plain", "Max pilots reached");
            return;
        }
    }

    uint32_t now = millis();
    PilotNode& pilot = pilots_[idx];

    uint32_t lap_time_ms = 0;
    if (pilot.lap_count == 0) {
        lap_time_ms = now - race_start_ms_;
    } else {
        lap_time_ms = now - pilot.last_lap_timestamp_ms;
    }

    if (lap_time_ms < 1000) lap_time_ms = 1000;
    if (lap_time_ms > 300000) lap_time_ms = 300000;

    pilot.last_lap_ms = lap_time_ms;
    pilot.lap_count++;
    pilot.total_time_ms += lap_time_ms;
    pilot.last_lap_timestamp_ms = now;

    if (pilot.best_lap_ms == 0 || lap_time_ms < pilot.best_lap_ms) {
        pilot.best_lap_ms = lap_time_ms;
    }

    pilot.active = true;
    data_changed_ = true;

    // Record in lap history (circular buffer, keep last MAX_LAP_HISTORY)
    if (lap_history_count_ < MAX_LAP_HISTORY) {
        lap_history_[lap_history_count_].lap_number = pilot.lap_count;
        lap_history_[lap_history_count_].lap_time_ms = lap_time_ms;
        lap_history_count_++;
    } else {
        // Shift left, add at end
        memmove(&lap_history_[0], &lap_history_[1], sizeof(LapRecord) * (MAX_LAP_HISTORY - 1));
        lap_history_[MAX_LAP_HISTORY - 1].lap_number = pilot.lap_count;
        lap_history_[MAX_LAP_HISTORY - 1].lap_time_ms = lap_time_ms;
    }

    // Track best overall
    if (best_overall_ms_ == 0 || lap_time_ms < best_overall_ms_) {
        best_overall_ms_ = lap_time_ms;
    }

    Serial.printf("[FPVGate] Pilot %s: Lap %d = %d.%03ds (best: %d.%03ds)\n",
                  pilot.name, pilot.lap_count,
                  lap_time_ms / 1000, lap_time_ms % 1000,
                  pilot.best_lap_ms / 1000, pilot.best_lap_ms % 1000);

    server_->send(200, "text/plain", "OK");
}

void FPVGateProvider::handle_race_start() {
    String client_ip = server_->client().remoteIP().toString();
    Serial.printf("[FPVGate] /RaceStart from %s\n", client_ip.c_str());

    race_active_ = true;
    race_start_ms_ = millis();

    // Reset lap history
    lap_history_count_ = 0;
    best_overall_ms_ = 0;
    memset(lap_history_, 0, sizeof(lap_history_));

    for (uint8_t i = 0; i < num_pilots_; i++) {
        pilots_[i].last_lap_ms = 0;
        pilots_[i].best_lap_ms = 0;
        pilots_[i].lap_count = 0;
        pilots_[i].total_time_ms = 0;
        pilots_[i].last_lap_timestamp_ms = race_start_ms_;
        pilots_[i].active = true;
    }

    data_changed_ = true;
    server_->send(200, "text/plain", "OK");
}

void FPVGateProvider::handle_race_stop() {
    String client_ip = server_->client().remoteIP().toString();
    Serial.printf("[FPVGate] /RaceStop from %s\n", client_ip.c_str());

    race_active_ = false;
    data_changed_ = true;
    server_->send(200, "text/plain", "OK");
}

void FPVGateProvider::handle_not_found() {
    String body = server_->arg("plain");
    Serial.printf("[FPVGate] Unknown: %s %s\n",
                  server_->method() == HTTP_POST ? "POST" : "GET",
                  server_->uri().c_str());
    Serial.printf("[FPVGate] Body: %s\n", body.c_str());
    for (int i = 0; i < server_->args(); i++) {
        Serial.printf("[FPVGate] Arg[%s] = %s\n", 
                      server_->argName(i).c_str(), 
                      server_->arg(i).c_str());
    }
    server_->send(404, "text/plain", "Not Found");
}

// ============================================================
// Pilot Management
// ============================================================

int8_t FPVGateProvider::find_pilot_by_ip(const char* ip) {
    for (uint8_t i = 0; i < num_pilots_; i++) {
        if (strcmp(pilots_[i].ip, ip) == 0) {
            return i;
        }
    }
    return -1;
}

int8_t FPVGateProvider::register_pilot(const char* ip) {
    if (num_pilots_ >= MAX_PILOTS) {
        Serial.println("[FPVGate] Cannot register pilot: max reached");
        return -1;
    }

    PilotNode& pilot = pilots_[num_pilots_];
    strncpy(pilot.ip, ip, 15);
    pilot.ip[15] = '\0';

    const char* last_dot = strrchr(ip, '.');
    uint8_t octet = last_dot ? atoi(last_dot + 1) : num_pilots_ + 1;
    snprintf(pilot.name, NAME_MAX_LEN + 1, "PILOT_%d", octet);

    pilot.last_lap_ms = 0;
    pilot.best_lap_ms = 0;
    pilot.lap_count = 0;
    pilot.total_time_ms = 0;
    pilot.last_lap_timestamp_ms = race_start_ms_;
    pilot.active = true;

    Serial.printf("[FPVGate] Registered pilot: %s (IP: %s)\n", pilot.name, ip);

    num_pilots_++;
    data_changed_ = true;
    return num_pilots_ - 1;
}

// ============================================================
// Single-Pilot Lap History
// ============================================================

uint8_t FPVGateProvider::get_lap_history(PilotEntry laps[], uint8_t max_count) {
    uint8_t count = (lap_history_count_ < max_count) ? lap_history_count_ : max_count;

    for (uint8_t i = 0; i < count; i++) {
        laps[i].position = lap_history_[i].lap_number;
        snprintf(laps[i].name, NAME_MAX_LEN + 1, "Lap %d", lap_history_[i].lap_number);
        laps[i].last_lap_ms = lap_history_[i].lap_time_ms;
        laps[i].best_lap_ms = best_overall_ms_;
        laps[i].lap_count = lap_history_[i].lap_number;
        // Gap = difference from best lap
        if (lap_history_[i].lap_time_ms == best_overall_ms_) {
            laps[i].gap_ms = -1;  // This IS the best lap
        } else {
            laps[i].gap_ms = (int32_t)(lap_history_[i].lap_time_ms - best_overall_ms_);
        }
        laps[i].total_time_ms = lap_history_[i].lap_time_ms;
    }

    return count;
}

// ============================================================
// Position Calculation
// ============================================================

void FPVGateProvider::recalculate_positions(PilotEntry output[], uint8_t count) {
    for (uint8_t i = 0; i < count - 1; i++) {
        for (uint8_t j = i + 1; j < count; j++) {
            bool swap = false;
            if (output[j].lap_count > output[i].lap_count) {
                swap = true;
            } else if (output[j].lap_count == output[i].lap_count &&
                       output[j].total_time_ms < output[i].total_time_ms) {
                swap = true;
            }
            if (swap) {
                PilotEntry temp = output[i];
                output[i] = output[j];
                output[j] = temp;
            }
        }
    }

    for (uint8_t i = 0; i < count; i++) {
        output[i].position = i + 1;
        if (i == 0) {
            output[i].gap_ms = -1;
        } else {
            output[i].gap_ms = (int32_t)(output[i].total_time_ms - output[0].total_time_ms);
        }
    }
}

#endif // NATIVE_BUILD
