#pragma once

enum SBUSChannels {
    WEAPONS_ENABLE = 0,
    AUTO_HAMMER_ENABLE = 1,
    HAMMER_CTRL = 2,
    FLAME_CTRL = 3,
    AUTO_SELF_RIGHT = 4,
    GENTLE_HAM_CTRL = 5,
    INTENSITY = 6,
    DANGER_MODE = 7,
    RANGE = 8,
    AUTOAIM_ENABLE = 9,
    HOLD_DOWN = 10,
    TURRET_SPIN = 9
};

// Boolean values coming in over RC are stored in a bitfield for ease of comparison
// to detect state changes.
enum RCBitfield {
    AUTO_HAMMER_ENABLE_BIT = 1,
    HAMMER_FIRE_BIT = 2,
    HAMMER_RETRACT_BIT = 4,
    FLAME_CTRL_BIT = 8,
    FLAME_PULSE_BIT = 16,
    GENTLE_HAM_F_BIT = 32,
    GENTLE_HAM_R_BIT = 64,
    // = 128,
    // = 256,
    DANGER_CTRL_BIT = 512,
    // = 1024,
    // = 2048,
    AUTO_AIM_ENABLE_BIT =  4096,
    // = 8192,
    // = 16384,
    WEAPONS_ENABLE_BIT = 32768
};

void initSBus();
void updateSBus();

bool isRadioConnected();

uint16_t getHammerIntensity();

int16_t desiredSBusTurretSpeed();

uint16_t getRange();

uint16_t getRcBitfield();

uint16_t getRcBitfieldChanges();

