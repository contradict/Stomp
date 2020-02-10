#ifndef WEAPONS_H
#define WEAPONS_H
#include "sbus.h"

#define RELATIVE_TO_FORWARD 221  // offset of axle anfle from 180 when hammer forward on floor. actual angle read 221
#define RELATIVE_TO_VERTICAL 32  // offset of axle angle from 90 when hammer arms vertical. actual angle read 122
#define RELATIVE_TO_BACK 25  // offset of axle angle from 0 when hammer back on floor. actual angle read 25
// RETRACT CONSTANTS
#define RETRACT_BEGIN_VEL_MAX 50.1f
static const uint32_t RETRACT_TIMEOUT = 2000 * 1000L;  // in microseconds
static const uint16_t RETRACT_COMPLETE_ANGLE = 20 + RELATIVE_TO_BACK;  // angle read  angle 53 off ground good on 4-09


bool weaponsEnabled();

void retract( bool check_velocity = true );

void fire( uint16_t hammer_intensity, bool flame_pulse, bool autofire );

void noAngleFire( uint16_t hammer_intensity, bool flame_pulse);

void gentleFire( RCBitfield control );

void gentleRetract( RCBitfield control );

void flameStart();

void flameEnd();

void valveSafe();

void valveEnable();

void flameSafe();

void flameEnable();

String startElectricHammerMove(int16_t speed);

void stopElectricHammerMove(void);

void enableState();

void safeState();

#endif // WEAPONS_H
