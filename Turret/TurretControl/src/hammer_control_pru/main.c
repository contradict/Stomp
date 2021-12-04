//
// This program runs on the PRU.  It initializes everything and then
// waits around to receive messages from the ARM processor.  This
// program also calls the hammer controller update method.
//
// Key Messages
//
// SENS - A message from the Sensor Controller.  Contains hammer and
//        turret angles and velocities as well as throw and retract
//        pressure reading.  Also contains hammer energy calculations
//        performed on ARM, derived from sensor readings.
//
// CONF - A message from either the Turret Controller or the Telemetry
//        Controller, initializing or updating global configuration
//        values used in controlling the hammer state machine
//
// THRW - Throw the hammer message
//
// Output Pins
//
// BBAI Header Pin P8.14 - PRU Heartbeat
// BBAI Header Pin P8.16 - unused
// BBAI Header Pin P8.18 - sensor message received
// BBAI Header Pin P8.15 - PRU 1 eCap PWM output
// 
// Input Pins
//

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pru_cfg.h>
#include <pru_ecap.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>

#include "pru_util.h"
#include "pru_time.h"

#include "hammer_control_pru/main.h"
#include "hammer_control_pru/rpmsg_handlers.h"
#include "hammer_control_pru/hammer_control_pru.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

static const uint32_t k_heartbeat_dt = 500000;

// directly write to gpio8_15 for heartbeat

static const uint32_t k_heartbeatPin = (0x1 << 15);

static uint32_t *k_gpio8OutputEnable = (uint32_t *)0x48053134;
static uint32_t *k_gpio8ClearDataOut = (uint32_t *)0x48053190;
static uint32_t *k_gpio8SetDataOut = (uint32_t *)0x48053194;


// -----------------------------------------------------------------------------
// states
// -----------------------------------------------------------------------------

enum turret_state
{
    turret_init,
    turret_sync,
    turret_safe,
    turret_armed,
    turret_active,
    turret_invalid = -1
};

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static enum turret_state s_state;

static uint32_t s_heartbeat_start_time;
static uint8_t s_radio_weapon_enabled = 0;


// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

void init();
void update();
void update_heartbeat();

void set_state(enum turret_state);

int8_t is_weapon_enabled();


// -----------------------------------------------------------------------------
//  Main
// -----------------------------------------------------------------------------

void main(void)
{
    // set initial state and init

    s_state = turret_invalid;
    set_state(turret_init);

    // execute forever

    while (1) 
    {
        update();
    }
}

void update()
{
    pru_time_update();

    // update state

    while (1)
    {
        enum turret_state prev_state = s_state;

        switch (s_state)
        {
            case turret_init:
                {
                    set_state(turret_sync);
                }
                break;

            case turret_sync:
                {
                    if (rpmsg_check_for_arm_sync())
                    {
                        set_state(turret_safe);
                    }
                }
                break;

            case turret_safe:
                {
                    if (rpmsg_check_for_arm_exit())
                    {
                        set_state(turret_sync);
                    }
                    else if (rpmsg_is_connected() && is_weapon_enabled())
                    {
                        set_state(turret_armed);
                    }
                }
                break;

            case turret_armed:
                {
                    if (!rpmsg_is_connected() || !is_weapon_enabled())
                    {
                        set_state(turret_safe);
                    }
                    else if (rpmsg_check_for_arm_exit())
                    {
                        set_state(turret_sync);
                    }
                }
                break;

            case turret_active:
                {
                    if (!rpmsg_is_connected() || !is_weapon_enabled())
                    {
                        set_state(turret_safe);
                    }
                    else if (hammer_control_is_swing_complete())
                    {
                        set_state(turret_armed);
                    }
                }
                break;
        }

        if (s_state == prev_state)
        {
            break;
        }
    }

    // now that state is updated, distribut the rest of the updates
    
    update_heartbeat();

    rpmsg_update();
    hammer_control_update();
}

void update_heartbeat()
{
    static uint8_t heartbeat_status = 0;

    uint32_t time = pru_time_gettime();

    if (time - s_heartbeat_start_time > k_heartbeat_dt)
    {
        s_heartbeat_start_time = time;

        uint32_t pins_high = 0x00;
        uint32_t pins_low = 0x00;

        if (heartbeat_status == 0)
        {
            heartbeat_status = 1;
            pins_high = k_heartbeatPin;
        }
        else
        {
            heartbeat_status = 0;
            pins_low = k_heartbeatPin;
        }

        (*k_gpio8ClearDataOut) = pins_low;
        (*k_gpio8SetDataOut) = pins_high;
    }
}

void request_hammer_swing()
{   
    //  If we are in armed state, that means we are ready and able to throw.
    //  tell the hammer swing state machine to throw, by setting our
    //  state to active

    if (s_state == turret_armed)
    {
        // TODO: Handle both types of swing
        set_state(turret_active);
    }
}

// -----------------------------------------------------------------------------
// Set State - tansition from one state to another
// -----------------------------------------------------------------------------

void set_state(enum turret_state new_state)
{
    if (s_state == new_state)
    {
        return;
    }

    // do work on leaving state

    switch (s_state)
    {
        case turret_active:
            {
                rpmsg_send_swng_message();
            }
            break;

        default:
            break;
    }

    // update to new state

    s_state = new_state;
 
    // do work on entering state
    
    switch (s_state)
    {
        case turret_init:
            {
                // init the pru time module

                pru_time_init();

                // init all our interal systems
                
                init();
                rpmsg_init();
                hammer_control_init();
            }
            break;

        case turret_active:
            {
                // throw the hammer!!!
                
                hammer_control_trigger_throw();
            }

        default:
            break;
    }
}

void init()
{
    // Allow OCP master port access by the PRU so the PRU can read external memories

    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0x0;


    // For the output enable register, 0 means eanbled and 1 is disabled
    // clear to zero just the 4 pins we need as output, leaving the rest 
    // as they are

    uint32_t enableOutput = ~k_heartbeatPin;
    (*k_gpio8OutputEnable) &= enableOutput;

    uint32_t pins_low = k_heartbeatPin;
    (*k_gpio8ClearDataOut) = pins_low;

    s_heartbeat_start_time = pru_time_gettime();
}

int8_t is_weapon_enabled()
{
    // Need to really implement this
    s_radio_weapon_enabled = 1;
    return s_radio_weapon_enabled;
}
