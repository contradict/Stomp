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
    TANK_PSI,
    HIGHEST_ID
} msg_id;
// Our msg_id has to be safely castable to a uint8_t
static_assert(HIGHEST_ID <= 256, "Too many messages!");

struct tank_psi {
    uint32_t psi;
} __attribute__((__packed__));