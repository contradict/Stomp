#ifndef PINS_H
#define PINS_H

//------------------ DEFINES----------------

// #define HARD_WIRED

// ----------------- ANALOG ----------------- 

// Sensors

#define PRESSURE_AI A15
#define ANGLE_AI A1
#define XBEE_CTS A12

// ----------------- DIGITAL  ---------------

// Mega2560 digital interrupt pins:
//
// 2 (int.0), 3 (int.1), 18 (int.5), 19 (int.4), 20 (int.3), 21 (int.2)
//
// We currently use 2 and 3 for these high-pri PWM inputs and leave the
// rest for other purposes

// BB MJS: Not used any more.  Controls for turret all come from the same raido now
// #define DRIVE_DISTANCE_PIN 2       // Drive radio ch6
// #define TARGETING_ENABLE_PIN 3     // Drive radio ch5

//  Hammer Pneumatics Control
//  BB MJS: These pins are not right yet

#define HAMMER_PNEUMATICS_ENABLE_VALVE 4

//  VERY IMPORTANT: In HammerController.cpp there is optimized code that requires
//  these pin asignments DONT change.  Chaning just here will break things.  

#define THROW_PRESSURE_VALVE_DO 6
#define THROW_VENT_VALVE_DO 7
#define RETRACT_PRESSURE_VALVE_DO 8
#define RETRACT_VENT_VALVE_DO 9

//  BB MJS: Old Pneumatics control

#define ENABLE_VALVE_DO 10
#define VENT_VALVE_DO 11
#define RETRACT_VALVE_DO A7
#define THROW_VALVE_DO 4

//  Flame thrower

#define IGNITER_DO 5
#define PROPANE_DO A6

// These are handled as pin change interrupts on the PINB bank

#define LEFT_RC_PIN 12             // Drive radio ch1
#define RIGHT_RC_PIN 13            // Drive radio ch2

// ----------------- GLOBALS ----------------

extern volatile bool g_enabled;

#endif
