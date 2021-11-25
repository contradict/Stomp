#pragma once

enum SBUSChannels {
    WEAPONS_ENABLE = 0,
    AUTO_HAMMER_ENABLE = 1,
    HAMMER_CTRL = 2,
    UNUSED_3 = 3,
    AUTO_SELF_RIGHT = 4,
    INTENSITY = 5,
    UNUSED_6 = 6,
    FLAME_RIGHT_CTRL = 7,
    RANGE = 8,
    TURRET_SPIN = 9,
    TURRET_CTL_MODE = 10
};

// Boolean values coming in over RC are stored in a bitfield for ease of comparison
// to detect state changes.
enum RCBitfield {
    AUTO_FIRE_ENABLE_BIT = 1,
    HAMMER_FIRE_BIT = 2,
    HAMMER_RETRACT_BIT = 4,
    FLAME_RIGHT_CTRL_BIT = 8,
    FLAME_RIGHT_PULSE_BIT = 16,
    FLAME_LEFT_CTRL_BIT = 32,
    FLAME_LEFT_PULSE_BIT = 64,
    AUTO_SELF_RIGHT_BIT = 128,
    // = 256,
    DANGER_CTRL_BIT = 512,
    // = 1024,
    // = 2048,
    AUTO_AIM_ENABLED_BIT =  4096,
    MANUAL_TURRET_BIT = 8192,
    // = 16384,
    WEAPONS_ENABLE_BIT = 32768
};

void initSBus();
void updateSBus();

bool isRadioConnected();
bool isWeaponEnabled();
bool isManualTurretEnabled();
bool isAutoAimEnabled();
bool isAutoFireEnabled();
bool isSelfRightEnabled();

bool isFlameRightOnEnabled();
bool isFlameRightPulseEnabled();
bool isFlameLeftOnEnabled();
bool isFlameLeftPulseEnabled();

bool hammerManualThrowAndRetract();
bool hammerManualRetractOnly();

uint16_t getRange();
int16_t getSwingFillAngle();
int16_t getDesiredManualTurretSpeed();

uint16_t getRcBitfield();
uint16_t getRcBitfieldChanges();

