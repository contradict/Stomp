//
// This file contains just the code to run the hammer state machine
//
// The important sensor data is placed in global variables from 
// the main program when it receives a SENS message.
//
// The important configuration values are also placed in global
// variables by the main program when it receives a CONF message.
//

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pru_util.h"
#include "pru_time.h"

#include "hammer_control/hammer_control.h"

// -----------------------------------------------------------------------------
// global variables
// -----------------------------------------------------------------------------

// sensor values (and thier derivatives) filled in from a SENS message from ARM
// in message handlling in main.c

int32_t g_hammer_angle;
int32_t g_hammer_velocity;
int32_t g_turret_angle;
int32_t g_turret_velocity;
int32_t g_throw_pressure;
int32_t g_retract_pressure;
int32_t g_hammer_energy;
int32_t g_available_break_energy;

// remote control radio values, filled in from COMM message from ARM
// in message handlling in main.c

int32_t g_desired_throw_angle; 

// config values, filled in from a CONF message from ARM
// in message handlling in main.c

int32_t g_max_throw_angle;
int32_t g_min_retract_angle;
int32_t g_retract_fill_pressure;
int32_t g_brake_exit_velocity;
int32_t g_emergency_brake_angle;
int32_t g_valve_change_dt;

int32_t g_max_throw_pressure_dt;
int32_t g_max_throw_expand_dt;
int32_t g_max_retract_pressure_dt;
int32_t g_max_retract_expand_dt;
int32_t g_max_retract_break_dt;
int32_t g_max_retract_settle_dt;

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// states
// -----------------------------------------------------------------------------

enum state
{
    init,
    idle,

    throw_setup,
    throw_pressurize,
    throw_expand,
    retract_setup,
    retract_pressurize,
    retract_expand,
    retract_brake,
    retract_settle,
    retract_complete,

    invalid = -1
};

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static enum state s_state = invalid;
static uint32_t s_state_start_time;

// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

void hammer_control_set_state(enum state new_state);

// -----------------------------------------------------------------------------
// public methods
// -----------------------------------------------------------------------------

void hammer_control_init()
{
    s_state = invalid;
    hammer_control_set_state(init);
}

void hammer_control_config_update()
{
    // if this is our first config, then we can now move on to the
    // rest of our state machine.  Otherwise, values are in globals
    // so they will just be used upon change
    
    if (s_state == init)
    {
        hammer_control_set_state(idle);
    }
}

void hammer_control_update()
{
    uint32_t time = pru_time_gettime();

    while (1)
    {
        enum state prev_state = s_state;

        switch (s_state)
        {
            case throw_setup:
                {
                    if (time - s_state_start_time > g_valve_change_dt)
                    {
                        hammer_control_set_state(throw_pressurize);
                    }
                }
                break;

            case throw_pressurize:
                {
                    if (g_hammer_angle >= g_desired_throw_angle ||
                        time - s_state_start_time > g_max_throw_pressure_dt)
                    {
                        hammer_control_set_state(throw_expand);
                    }
                }
                break;

            case throw_expand:
                {
                    if (g_hammer_angle >= g_desired_throw_angle ||
                        time - s_state_start_time > g_max_throw_expand_dt)
                    {
                        hammer_control_set_state(retract_setup);
                    }
                }
                break;

            case retract_setup:
                {
                    if (time - s_state_start_time > g_valve_change_dt)
                    {
                        hammer_control_set_state(retract_pressurize);
                    }
                }
                break;

            case retract_pressurize:
                {
                    if (g_retract_pressure >= g_retract_fill_pressure ||
                        g_hammer_angle <= g_emergency_brake_angle ||
                        time - s_state_start_time > g_max_retract_pressure_dt)
                    {
                        hammer_control_set_state(retract_expand);
                    }
                }
                break;

            case retract_expand:
                {
                    if (g_hammer_energy >= g_available_break_energy ||
                        g_hammer_angle <= g_emergency_brake_angle ||
                        time - s_state_start_time > g_max_retract_expand_dt)
                    {
                        hammer_control_set_state(retract_brake);
                    }
                }
                break;

            case retract_brake:
                {
                    if (g_hammer_velocity <= g_brake_exit_velocity ||
                        time - s_state_start_time > g_valve_change_dt)
                    {
                        hammer_control_set_state(retract_settle);
                    }
                }
                break;

            case retract_settle:
                {
                    if (g_hammer_angle <= g_min_retract_angle ||
                        time - s_state_start_time > g_valve_change_dt)
                    {
                        hammer_control_set_state(retract_complete);
                    }
                }
                break;

            case retract_complete:
                {
                    if (time - s_state_start_time > g_valve_change_dt)
                    {
                        hammer_control_set_state(idle);
                    }
                }
                break;
        }

        if (s_state == prev_state)
        {
            break;
        }
    }
}

uint8_t hammer_control_is_swing_complete()
{
    return s_state == idle;
}

// -----------------------------------------------------------------------------
// private methods
// -----------------------------------------------------------------------------

void hammer_control_set_state(enum state new_state)
{
    if (new_state == s_state)
    {
        return;
    }

    // do work on leaving state

    switch (s_state)
    {
        case init:
            {
            }
            break;
    }

    // update to new state
    
    s_state = new_state;
    s_state_start_time = pru_time_gettime();

    // do work on entering state
    
    switch (s_state)
    {
        case init:
            {
            }
            break;
    };
}

