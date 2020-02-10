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
// 2 (int.0), 3 (int.1), 18 (int.5), 19 (int.4), 20 (int.3), 21 (int.2)
// We currently use 2 and 3 for these high-pri PWM inputs and leave the
// rest for other purposes
#define DRIVE_DISTANCE_PIN 2       // Drive radio ch6
#define TARGETING_ENABLE_PIN 3     // Drive radio ch5

#define ENABLE_VALVE_DO 10
#define VENT_VALVE_DO 11

#define IGNITER_DO 5
#define PROPANE_DO A6

#define RETRACT_VALVE_DO A7
#define THROW_VALVE_DO 4

#define SELF_RIGHT_RIGHT_EXTEND_DO 8
#define SELF_RIGHT_RIGHT_RETRACT_DO 9
#define SELF_RIGHT_LEFT_EXTEND_DO A10
#define SELF_RIGHT_LEFT_RETRACT_DO A11

#define VACUUM_VALVE_DO A8
#define AUX1_2A_DO 6
#define AUX2_2A_DO 7
#define AUX3_2A_DO A9

#define VACUUM_AI_LEFT A2
#define VACUUM_AI_RIGHT A3

// These are handled as pin change interrupts on the PINB bank
#define LEFT_RC_PIN 12             // Drive radio ch1
#define RIGHT_RC_PIN 13            // Drive radio ch2

// Drive control output (for CAN testing)
// #define CHIP_SELECT_PIN 19

// ----------------- GLOBALS ----------------
extern volatile bool g_enabled;

#endif
