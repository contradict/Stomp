#ifdef TURRET
#include <inttypes.h>
#include <unistd.h>
#include <lcm/lcm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>

#include "messages.h"
#include "telemetry.h"
#include "sclog4c/sclog4c.h"
#include "utils/utils.h"

#include "lcm_handlers.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

const int turret_period_msec = 200; // turret packet send period
const int turret_sensors_period_msec = 20; // turret packet send period

// -----------------------------------------------------------------------------
// lcm_message_handlers
// -----------------------------------------------------------------------------

void turret_telemetry_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_turret_telemetry *msg, void *user)
{
    static struct timeval now;
    static struct timeval last_send_time =  (struct timeval){0};

    gettimeofday(&now, 0);
    time_t millis = time_diff_msec(last_send_time, now);
    logm(SL4C_FINE, "%ld msec since last %s msg received", millis, channel);

    if (millis < 0 || millis > turret_period_msec) //prepare and send COSMOS msg
    {
        gettimeofday(&last_send_time, 0);
        struct turret_gnrl_cosmos cosmos_msg;
        logm(SL4C_DEBUG, "Sending COSMOS Turret General packet");

        cosmos_msg.turret_state = msg->turret_state;
        cosmos_msg.rotation_state = msg->rotation_state;

        telem_publish(TURRET_GNRL, (char *)&cosmos_msg, sizeof(cosmos_msg));
    }
}

void turret_sensors_control_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_sensors_control *msg, void *user)
{
    static struct timeval now;
    static struct timeval last_send_time = (struct timeval){0};

    gettimeofday(&now, 0);

    time_t millis = time_diff_msec(last_send_time, now);
    logm(SL4C_FINE, "%ld msec since last %s msg received", millis, channel);

    if (millis < 0 || millis > turret_sensors_period_msec) //prepare and send COSMOS msg
    {
        gettimeofday(&last_send_time, 0);
        logm(SL4C_DEBUG, "Sending COSMOS Turret Sensors packet");

        struct turret_sensors_cosmos cosmos_msg;

        cosmos_msg.hammer_angle = msg->hammer_angle;
        cosmos_msg.hammer_velocity = msg->hammer_velocity;
        cosmos_msg.hammer_energy = msg->hammer_energy;
        cosmos_msg.available_break_energy = msg->available_break_energy;
        cosmos_msg.turret_angle = msg->turret_angle;
        cosmos_msg.turret_velocity = msg->turret_velocity;
        cosmos_msg.throw_pressure = msg->throw_pressure;
        cosmos_msg.retract_pressure = msg->retract_pressure;            

        telem_publish(TURRET_SNS, (char *)&cosmos_msg, sizeof(cosmos_msg));
    }
}

void turret_hammer_swing_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_hammer_swing *msg, void *user)
{
    logm(SL4C_DEBUG, "Sending COSMOS Turret Hammer Swing packet\n\tdt:%d, tv:%d, tl:%d, tr:%d, sf:%d, st:%d", 
        msg->swing_state_dt,
        msg->trigger_value,
        msg->trigger_limit,
        msg->trigger_reason,
        msg->swing_state_from,
        msg->swing_state_to);

    struct turret_hammer_swing_cosmos cosmos_msg;

    cosmos_msg.swing_state_dt = msg->swing_state_dt;
    cosmos_msg.trigger_value = msg->trigger_value;
    cosmos_msg.trigger_limit = msg->trigger_limit;
    cosmos_msg.trigger_reason = msg->trigger_reason;
    cosmos_msg.swing_state_from = msg->swing_state_from;
    cosmos_msg.swing_state_to = msg->swing_state_to;

    telem_publish(TURRET_SWING, (char *)&cosmos_msg, sizeof(cosmos_msg));
}

#endif // #ifdef TURRET
