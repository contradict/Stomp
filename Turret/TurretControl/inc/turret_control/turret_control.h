#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include <lcm/lcm.h>
#include <toml.h>

// radio control parameters

enum turret_enable 
{
    TURRET_DISABLED,
    TURRET_ENABLED
};

enum turret_rotation_mode 
{
    ROTATION_MODE_DISABLED,
    ROTATION_MODE_MANUAL,
    ROTATION_MODE_AUTO
};

enum hammer_control_trigger
{
    HAMMER_SAFE,
    HAMMER_TRIGGER_THROW,
    HAMMER_TRIGGER_RETRACT,
};

struct radio_control_parameters_t
{
    float rotation_intensity;
    float throw_intensity;
    float retract_intensity;

    enum turret_enable enable;
    enum turret_rotation_mode rotation_mode;
    enum hammer_control_trigger hammer_trigger;
};

// Global Variables

extern lcm_t *g_lcm;
extern struct radio_control_parameters_t g_radio_control_parameters;

