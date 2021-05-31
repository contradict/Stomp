#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

//  config values that we need to send down to the pru

struct pru_config
{
    int64_t max_throw_angle;
    int64_t min_retract_angle;
    int64_t min_retract_fill_pressure;
    int64_t break_exit_velocity;
    int64_t emergency_break_angle;
    int64_t valve_change_dt;
    int64_t max_throw_pressure_dt;
    int64_t max_throw_expand_dt;
    int64_t max_retract_pressure_dt;
    int64_t max_retract_expand_dt;
    int64_t max_retract_break_dt;
    int64_t max_retract_settle_dt;
};

extern struct pru_config g_pru_config;
