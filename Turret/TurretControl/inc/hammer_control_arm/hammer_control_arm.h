#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <lcm/lcm.h>

//  config values that we need to send down to the pru

struct pru_config
{
    float max_throw_angle;
    float min_retract_angle;
    float break_exit_velocity;
    float emergency_break_angle;
    int32_t valve_change_dt;
    int32_t max_throw_pressure_dt;
    int32_t max_throw_expand_dt;
    int32_t max_retract_pressure_dt;
    int32_t max_retract_expand_dt;
    int32_t max_retract_break_dt;
    int32_t max_retract_settle_dt;
};

extern lcm_t *g_lcm;
extern int g_rpmsg_fd;
extern struct pru_config g_pru_config;

// Extens for all the LCM Channels so they can be used by other (than main.c) files.

extern const char* SBUS_RADIO_COMMAND;
extern const char* SENSORS_CONTROL;
extern const char* TURRET_TELEMETRY;
extern const char* HAMMER_TRIGGER;
extern const char* HAMMER_SWING;
extern const char* TLM_CMD_HAMMER_CONF;

