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

#include "hammer_control_pru/rpmsg_handlers.h"
#include "hammer_control_pru/hammer_control_pru.h"

// -----------------------------------------------------------------------------
// global variables
// -----------------------------------------------------------------------------

// remote control values, filled in from FIRE message from ARM in message
// handelling in rpmsg_handlers.c

int32_t g_throw_desired_intensity = 0;
int32_t g_retract_desired_intensity = 0;

// sensor values (and thier derivatives) filled in from a SENS message from ARM
// in message handlling in rpmsg_handlers.c

int32_t g_hammer_angle = 0;
int32_t g_hammer_velocity = 0;
int32_t g_turret_angle = 0;
int32_t g_turret_velocity = 0;
int32_t g_throw_pressure = 0;
int32_t g_retract_pressure = 0;
int32_t g_hammer_energy = 0;
int32_t g_available_break_energy = 0;

// config values, filled in from a CONF message from ARM
// in message handlling in rpmsg_handlers.c

int64_t g_max_throw_angle;
int64_t g_min_retract_angle;
int64_t g_brake_exit_velocity;
int64_t g_emergency_brake_angle;
int64_t g_valve_change_dt;

int64_t g_max_throw_pressure_dt;
int64_t g_max_throw_expand_dt;
int64_t g_max_retract_pressure_dt;
int64_t g_max_retract_expand_dt;
int64_t g_max_retract_break_dt;
int64_t g_max_retract_settle_dt;

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

// Hammer valves are controled by these pins
//
// P8.35a (gpio8_12): Throw Pressure Valve
// P8.36a (gpio8_10): Throw Vent Valve
// P8.33a (gpio8_13): Retract Pressure Valve
// P8.34a (gpio8_11): Retract Vent Valve
//
// So, we need to get the control register address for gpio8
// for channel 10, 11, 12, and 13

static uint32_t *k_gpio8OutputEnable = (uint32_t *)0x48053134;
static uint32_t *k_gpio8ClearDataOut = (uint32_t *)0x48053190;
static uint32_t *k_gpio8SetDataOut = (uint32_t *)0x48053194;

static const uint32_t k_throwPressurePin = (0x1 << 12);
static const uint32_t k_throwVentPin = (0x1 << 10);
static const uint32_t k_retractPressurePin = (0x1 << 13);
static const uint32_t k_retractVentPin = (0x1 << 11);


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
static uint8_t s_initial_config_received = 0;

// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

void hammer_control_set_state(enum state new_state);
void hammer_control_set_safe();

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
    s_initial_config_received = 1;
}

void hammer_control_trigger_throw(int32_t desired_throw_angle)
{
    // Only start a hammer swing if we are currently idle
    
    if (s_state != idle)
    {
        return;
    }

    hammer_control_set_state(throw_setup);
}

void hammer_control_trigger_retract()
{
    // Only start a hammer retract if we are currently idle
    
    if (s_state != idle)
    {
        return;
    }

    hammer_control_set_state(retract_setup);
}

void hammer_control_update()
{
    while (1)
    {
        enum state prev_state = s_state;

        uint32_t time = pru_time_gettime();
        int32_t state_dt = time - s_state_start_time;

        switch (s_state)
        {
            case init:
                {
                    if (s_initial_config_received)
                    {
                        hammer_control_set_state(idle);
                    }
                }
                break;

            case throw_setup:
                {
                    if (state_dt > g_valve_change_dt)
                    {
                        hammer_control_set_state(throw_pressurize);
                        rpmsg_send_swing_message(state_dt, state_dt, g_valve_change_dt, timeout, throw_setup, throw_pressurize);
                    }
                }
                break;

            case throw_pressurize:
                {
                    if (g_hammer_angle >= g_throw_desired_intensity)
                    {
                        hammer_control_set_state(throw_expand);
                        rpmsg_send_swing_message(state_dt, g_hammer_angle, g_throw_desired_intensity, hammer_angel_greater, throw_pressurize, throw_expand);
                    }
                    else if (state_dt > g_max_throw_pressure_dt)
                    {
                        hammer_control_set_state(throw_expand);
                        rpmsg_send_swing_message(state_dt, state_dt, g_max_throw_pressure_dt, timeout, throw_pressurize, throw_expand);
                    }
                }
                break;

            case throw_expand:
                {
                    if (g_hammer_angle >= g_max_throw_angle)
                    {
                        hammer_control_set_state(retract_setup);
                        rpmsg_send_swing_message(state_dt, g_hammer_angle, g_max_throw_angle, hammer_angel_greater, throw_expand, retract_setup);
                    }
                    else if (state_dt > g_max_throw_expand_dt)
                    {
                        hammer_control_set_state(retract_setup);
                        rpmsg_send_swing_message(state_dt, state_dt, g_max_throw_expand_dt, timeout, throw_expand, retract_setup);
                    }
                }
                break;

            case retract_setup:
                {
                    if (state_dt > g_valve_change_dt)
                    {
                        hammer_control_set_state(retract_pressurize);
                        rpmsg_send_swing_message(state_dt, state_dt, g_valve_change_dt, timeout, retract_setup, retract_pressurize);
                    }
                }
                break;

            case retract_pressurize:
                {
                    if (g_retract_pressure >= g_retract_desired_intensity)
                    {
                        hammer_control_set_state(retract_expand);
                        rpmsg_send_swing_message(state_dt, g_retract_pressure, g_retract_desired_intensity, retract_pressure_greater, retract_pressurize, retract_expand);
                    }
                    else if (g_hammer_angle <= g_emergency_brake_angle)
                    {
                        hammer_control_set_state(retract_expand);
                        rpmsg_send_swing_message(state_dt, g_hammer_angle, g_emergency_brake_angle, hammer_angel_less, retract_pressurize, retract_expand);
                    }
                    else if (state_dt > g_max_retract_pressure_dt)
                    {
                        hammer_control_set_state(retract_expand);
                        rpmsg_send_swing_message(state_dt, state_dt, g_valve_change_dt, timeout, retract_pressurize, retract_expand);
                    }
                }
                break;

            case retract_expand:
                {
                    if (g_hammer_energy >= g_available_break_energy)
                    {
                        hammer_control_set_state(retract_brake);
                        rpmsg_send_swing_message(state_dt, g_hammer_energy, g_available_break_energy, hammer_energy_greater, retract_expand, retract_brake);
                    }
                    else if (g_hammer_angle <= g_emergency_brake_angle)
                    {
                        hammer_control_set_state(retract_brake);
                        rpmsg_send_swing_message(state_dt, g_hammer_angle, g_emergency_brake_angle, hammer_angel_less, retract_expand, retract_brake);
                    }
                    else if (state_dt > g_max_retract_expand_dt)
                    {
                        hammer_control_set_state(retract_brake);
                        rpmsg_send_swing_message(state_dt, state_dt, g_max_retract_expand_dt, timeout, retract_expand, retract_brake);
                    }
                }
                break;

            case retract_brake:
                {
                    if (abs(g_hammer_velocity) <= abs(g_brake_exit_velocity))
                    {
                        hammer_control_set_state(retract_settle);
                        rpmsg_send_swing_message(state_dt, g_hammer_velocity, g_brake_exit_velocity, hammer_velocity_less, retract_brake, retract_settle);
                    }
                    else if (state_dt > g_max_retract_break_dt)
                    {
                        hammer_control_set_state(retract_settle);
                        rpmsg_send_swing_message(state_dt, state_dt, g_max_retract_break_dt, timeout, retract_brake, retract_settle);
                    }
                }
                break;

            case retract_settle:
                {
                    if (g_hammer_angle <= g_min_retract_angle)
                    {
                        hammer_control_set_state(retract_complete);
                        rpmsg_send_swing_message(state_dt, g_hammer_angle, g_min_retract_angle, timeout, retract_settle, retract_complete);
                    }
                    else if (state_dt > g_max_retract_settle_dt)
                    {
                        hammer_control_set_state(retract_complete);
                        rpmsg_send_swing_message(state_dt, state_dt, g_max_retract_settle_dt, timeout, retract_settle, retract_complete);
                    }
                }
                break;

            case retract_complete:
                {
                    if (state_dt > g_valve_change_dt)
                    {
                        hammer_control_set_state(idle);
                        rpmsg_send_swing_message(state_dt, state_dt, g_valve_change_dt, timeout, retract_complete, idle);
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
                // For the output enable register, 0 means eanbled and 1 is disabled
                // clear to zero just the 4 pins we need as output, leaving the rest 
                // as they are

                uint32_t enableOutput =  k_throwPressurePin | k_throwVentPin | k_retractPressurePin | k_retractVentPin;
                enableOutput = ~enableOutput;
                (*k_gpio8OutputEnable) &= enableOutput;

                hammer_control_set_safe();
            }
            break;

        case idle:
            {
                // Throw Pressure: Closed (signal low)
                // Throw Vent: Open (signal low)
                // Retract Pressure: Closed (signal low)
                // Retract Vent: Open (signal low)

                hammer_control_set_safe();
            }
            break;

        case throw_setup:
            {
                // Throw Pressure: Closed (signal low)
                // Throw Vent: Closed (signal high)
                // Retract Pressure: Closed (signal low)
                // Retract Vent: Closed (signal high)

                uint32_t pins_high = k_throwVentPin | k_retractVentPin;
                uint32_t pins_low = k_throwPressurePin | k_retractPressurePin;

                (*k_gpio8ClearDataOut) = pins_low;
                (*k_gpio8SetDataOut) = pins_high;
            }
            break;

        case throw_pressurize:
            {
                // Throw Pressure: Open (signal high)
                // Throw Vent: Closed (signal high)
                // Retract Pressure: Closed (signal low)
                // Retract Vent: Closed (signal high)

                uint32_t pins_high = k_throwPressurePin | k_throwVentPin | k_retractVentPin;
                uint32_t pins_low = k_retractPressurePin;

                (*k_gpio8ClearDataOut) = pins_low;
                (*k_gpio8SetDataOut) = pins_high;
            }
            break;

        case throw_expand:
            {
                // Throw Pressure: Closed (signal low)
                // Throw Vent: Closed (signal high)
                // Retract Pressure: Closed (signal low)
                // Retract Vent: Closed (signal high)

                uint32_t pins_high = k_throwVentPin | k_retractVentPin;
                uint32_t pins_low = k_throwPressurePin | k_retractPressurePin;

                (*k_gpio8ClearDataOut) = pins_low;
                (*k_gpio8SetDataOut) = pins_high;
            }
            break;

        case retract_setup:
            {
                // Throw Pressure: Closed (signal low)
                // Throw Vent: Open (signal low)
                // Retract Pressure: Closed (signal low)
                // Retract Vent: Closed (signal high)

                uint32_t pins_high = k_retractVentPin;
                uint32_t pins_low = k_throwPressurePin | k_throwVentPin | k_retractPressurePin;

                (*k_gpio8ClearDataOut) = pins_low;
                (*k_gpio8SetDataOut) = pins_high;
            }
            break;

        case retract_pressurize:
            {
                // Throw Pressure: Closed (signal low)
                // Throw Vent: Open (signal low)
                // Retract Pressure: Open (signal high)
                // Retract Vent: Closed (signal high)

                uint32_t pins_high = k_retractPressurePin | k_retractVentPin;
                uint32_t pins_low = k_throwPressurePin | k_throwVentPin;

                (*k_gpio8ClearDataOut) = pins_low;
                (*k_gpio8SetDataOut) = pins_high;
            }
            break;

        case retract_expand:
            {
                // Throw Pressure: Closed (signal low)
                // Throw Vent: Open (signal low)
                // Retract Pressure: Closed (signal low)
                // Retract Vent: Closed (signal high)

                uint32_t pins_high = k_retractVentPin;
                uint32_t pins_low = k_throwPressurePin | k_throwVentPin | k_retractPressurePin;

                (*k_gpio8ClearDataOut) = pins_low;
                (*k_gpio8SetDataOut) = pins_high;
            }
            break;

        case retract_brake:
            {
                // Throw Pressure: Closed (signal low)
                // Throw Vent: Closed (signal high)
                // Retract Pressure: Closed (signal low)
                // Retract Vent: Closed (signal high)

                uint32_t pins_high = k_throwVentPin | k_retractVentPin;
                uint32_t pins_low = k_throwPressurePin | k_retractPressurePin;

                (*k_gpio8ClearDataOut) = pins_low;
                (*k_gpio8SetDataOut) = pins_high;
            }
            break;

        case retract_settle:
            {
                // Throw Pressure: Closed (signal low)
                // Throw Vent: Opened (signal low)
                // Retract Pressure: Closed (signal low)
                // Retract Vent: Closed (signal high)

                uint32_t pins_high = k_retractVentPin;
                uint32_t pins_low = k_throwPressurePin | k_throwVentPin | k_retractPressurePin;

                (*k_gpio8ClearDataOut) = pins_low;
                (*k_gpio8SetDataOut) = pins_high;
            }
            break;

        case retract_complete:
            {
                // Throw Pressure: Closed (signal low)
                // Throw Vent: Open (signal low)
                // Retract Pressure: Closed (signal low)
                // Retract Vent: Open (signal low)

                hammer_control_set_safe();
            }
            break;
    };
}

void hammer_control_set_safe() 
{
    // Turn all hammer control valves to off (low), this is the safe mode 
    //
    // Throw Pressure: Closed (signal low)
    // Throw Vent: Open (signal low)
    // Retract Pressure: Closed (signal low)
    // Retract Vent: Open (signal low)

    uint32_t pins_low = k_throwPressurePin | k_throwVentPin | k_retractPressurePin | k_retractVentPin;
    (*k_gpio8ClearDataOut) = pins_low;
}

