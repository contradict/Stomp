/*
    messages.h
    Shared header for message structures used by multiple modules, e.g. a sensor driver 
    and the telemetry system. 
*/
#pragma once
#include <assert.h>
#include <stdint.h>

// This is just a convenient define for sizing buffers, not based on
// a hardware limitation.
#define MAX_MESSAGE_SIZE 256

// All messages should have an entry in this enum
// N.B: Reordering these IDs may have consequences for e.g. Cosmos telemetry files.
// In a better world there would be autogeneration to eliminate this pitfall.
typedef enum msg_id {
    INVALID = 0,  // Avoid having a meaningful ID of 0
    GNRL,
    LEGSTAT,
    TANK_PSI,
    HIGHEST_ID
} msg_id;
// Our msg_id has to be safely castable to a uint8_t
static_assert(HIGHEST_ID <= 256, "Too many messages!");

struct gnrl_cosmos {
    uint8_t tank_psi;
    uint8_t rail_psi;
    uint8_t flags_byte;
//  unsigned int sbus_failsafe : 1;
//  unsigned int sbus_no_data : 1;
} __attribute__((__packed__));

struct legs_cosmos {
    int8_t curl_base_psi[6];
    int8_t curl_rod_psi[6];
    int8_t swing_base_psi[6];
    int8_t swing_rod_psi[6];
    int8_t lift_base_psi[6];
    int8_t lift_rod_psi[6];
    int8_t toe_pos_x[6];
    int8_t toe_cmd_x[6];
    int8_t toe_pos_y[6];
    int8_t toe_cmd_y[6];
    int8_t toe_pos_z[6];
    int8_t toe_cmd_z[6];
    uint8_t observed_period;
} __attribute__((__packed__));

struct tank_psi {
    uint32_t psi;
} __attribute__((__packed__));
