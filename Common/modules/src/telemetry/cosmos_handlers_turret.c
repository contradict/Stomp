#ifdef TURRET

#include <inttypes.h>
#include <unistd.h>
#include <lcm/lcm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <math.h>

#include "main.h"
#include "cosmos_handlers.h"
#include "lcm/stomp_tlm_cmd_hammer_conf.h"

#include "sclog4c/sclog4c.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// cosmos message handlers
// -----------------------------------------------------------------------------

void hammer_conf_handler(char *message)
{
    cosmos_hammer_conf_msg cosmos_msg;
    stomp_tlm_cmd_hammer_conf lcm_msg;

    memcpy(&cosmos_msg, message, sizeof(cosmos_hammer_conf_msg));

    lcm_msg.max_throw_angle = cosmos_msg.max_throw_angle * M_PI/180.0f;
    lcm_msg.min_retract_angle = cosmos_msg.min_retract_angle * M_PI/180.0f;
    lcm_msg.break_exit_velocity = cosmos_msg.break_exit_velocity * M_PI/180.0f;
    lcm_msg.emergency_break_angle = cosmos_msg.emergency_break_angle * M_PI/180.0f;
    lcm_msg.valve_change_dt = (int32_t)(cosmos_msg.valve_change_dt * 1000000.0f);
    lcm_msg.max_throw_pressure_dt = (int32_t)(cosmos_msg.max_throw_pressure_dt * 1000000.0f);
    lcm_msg.max_throw_expand_dt = (int32_t)(cosmos_msg.max_throw_expand_dt * 1000000.0f);
    lcm_msg.max_retract_pressure_dt = (int32_t)(cosmos_msg.max_retract_pressure_dt * 1000000.0f);
    lcm_msg.max_retract_expand_dt = (int32_t)(cosmos_msg.max_retract_expand_dt  * 1000000.0f);
    lcm_msg.max_retract_break_dt = (int32_t)(cosmos_msg.max_retract_break_dt  * 1000000.0f);
    lcm_msg.max_retract_settle_dt = (int32_t)(cosmos_msg.max_retract_settle_dt * 1000000.0f);

    stomp_tlm_cmd_hammer_conf_publish(g_lcm, TLM_CMD_HAMMER_CONF, &lcm_msg);
}

#endif // #ifdef TURRET
