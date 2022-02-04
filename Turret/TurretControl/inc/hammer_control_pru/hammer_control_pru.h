#pragma once

// -----------------------------------------------------------------------------
// global variables
// -----------------------------------------------------------------------------

// IMPORTANT: All values are fixed point representation and are converted to and
//            from float values by 'hammer_control_arm'.  'hammer_control_pru'
//            just used these values in compairisons and is never aware of what
//            the individual format or units are.  
//
//            The only exception is variables that represent delta time.  In this
//            case, hammer_control_pru requires these values to be in microseconds. 
//
//            This is true for Sensor Values, Config Values and
//            Remote Control Values

// sensor values (and thier derivatives) filled in from a SENS message from ARM

extern int32_t g_hammer_angle;
extern int32_t g_hammer_velocity;
extern int32_t g_hammer_energy;
extern int32_t g_available_break_energy;
extern int32_t g_turret_angle;
extern int32_t g_turret_velocity;
extern int32_t g_throw_pressure;
extern int32_t g_retract_pressure;

// config values, filled in from a CONF message from ARM

extern int64_t g_max_throw_angle;
extern int64_t g_min_retract_angle;
extern int64_t g_brake_exit_velocity;
extern int64_t g_emergency_brake_angle;
extern int64_t g_valve_change_dt;
extern int64_t g_max_throw_pressure_dt;
extern int64_t g_max_throw_expand_dt;
extern int64_t g_max_retract_pressure_dt;
extern int64_t g_max_retract_expand_dt;
extern int64_t g_max_retract_break_dt;
extern int64_t g_max_retract_settle_dt;

// control radio values, filled in from FIRE message from ARM

extern int32_t g_throw_desired_intensity;
extern int32_t g_retract_desired_intensity;

// -----------------------------------------------------------------------------
// public methods
// -----------------------------------------------------------------------------

void hammer_control_init();
void hammer_control_update();
void hammer_control_config_update();

void hammer_control_trigger_throw();
void hammer_control_trigger_retract();

uint8_t hammer_control_is_swing_complete();
