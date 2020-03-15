/*
    messages.h
    Shared header for message structures used by multiple modules, e.g. a sensor driver 
    and the telemetry system. 
*/
#pragma once
#include <stdint.h>

// All messages should have an entry in this enum
// N.B: Reordering these IDs may have consequences for e.g. Cosmos telemetry files.
// In a better world there would be autogeneration to eliminate this pitfall.
typedef enum msg_id {
    TANK_PSI = 0,
} msg_id;

struct tank_psi {
    uint32_t psi;
} __attribute__((__packed__));