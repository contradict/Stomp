#pragma once

// -----------------------------------------------------------------------------
// global variables
// -----------------------------------------------------------------------------

// sensor values (and thier derivatives) filled in from a SENS message from ARM

extern int32_t g_hammer_angle;
extern int32_t g_hammer_velocity;
extern int32_t g_turret_angle;
extern int32_t g_turret_velocity;
extern int32_t g_throw_pressure;
extern int32_t g_retract_pressure;
extern int32_t g_hammer_energy;
extern int32_t g_available_break_energy;

// remote control radio values, filled in from COMM message from ARM

extern int32_t g_desired_throw_angle; 

// config values, filled in from a CONF message from ARM

extern int32_t g_max_throw_angle;
extern int32_t g_min_retract_angle;
extern int32_t g_retract_fill_pressure;
extern int32_t g_brake_exit_velocity;
extern int32_t g_emergency_brake_angle;
extern int32_t g_valve_change_dt;

extern int32_t g_max_throw_pressure_dt;
extern int32_t g_max_throw_expand_dt;
extern int32_t g_max_retract_pressure_dt;
extern int32_t g_max_retract_expand_dt;
extern int32_t g_max_retract_break_dt;
extern int32_t g_max_retract_settle_dt;

// -----------------------------------------------------------------------------
// public methods
// -----------------------------------------------------------------------------

void hammer_control_init();
void hammer_control_update();
void hammer_control_config_update();

uint8_t hammer_control_is_swing_complete();
