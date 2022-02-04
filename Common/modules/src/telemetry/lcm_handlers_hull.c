#ifdef HULL
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

const int gnrl_period_msec = 200;  // general packet send period
const int leg_period_msec = 200; // leg packet send period

// -----------------------------------------------------------------------------
// lcm_message_handlers
// -----------------------------------------------------------------------------

int8_t m_to_int(float value){
    return (int8_t) value;
}

int8_t psi_to_int(float value){
    return (int8_t) value;
}

void leg_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_telemetry_leg *msg, void *user)
{
    static struct timeval now;
    static struct timeval last_send_time;

    gettimeofday(&now, 0);
    int millis = time_diff_msec(last_send_time, now);
    logm(SL4C_FINE, "%d msec since last %s msg received", millis, channel);

    if (millis < 0 || millis > leg_period_msec) //prepare and send COSMOS msg
    {
        gettimeofday(&last_send_time, 0);
        struct legs_cosmos cosmos_msg;
        logm(SL4C_DEBUG, "Sending COSMOS LEGS packet");
        int leg;
        for (leg = 0; leg < STOMP_TELEMETRY_LEG_LEGS; leg++)
        {
            int curl = leg + STOMP_TELEMETRY_LEG_CURL;
            int swing = leg + STOMP_TELEMETRY_LEG_SWING;
            int lift = leg + STOMP_TELEMETRY_LEG_LIFT;
            cosmos_msg.curl_base_psi[leg] = psi_to_int(msg->base_end_pressure[curl]);
            cosmos_msg.curl_rod_psi[leg] = psi_to_int(msg->rod_end_pressure[curl]);
            cosmos_msg.swing_base_psi[leg] = psi_to_int(msg->base_end_pressure[swing]);
            cosmos_msg.swing_rod_psi[leg] = psi_to_int(msg->rod_end_pressure[swing]);
            cosmos_msg.lift_base_psi[leg] = psi_to_int(msg->base_end_pressure[lift]);
            cosmos_msg.lift_rod_psi[leg] = psi_to_int(msg->rod_end_pressure[lift]);
            cosmos_msg.toe_pos_x[leg] = m_to_int(msg->toe_position_measured_X[leg]);
            logm(SL4C_DEBUG, "Leg: %d, toe_x_pos: %.4f, cosmos: %d", leg, msg->toe_position_measured_X[leg], cosmos_msg.toe_pos_x[leg]);
            cosmos_msg.toe_cmd_x[leg] = m_to_int(msg->toe_position_commanded_X[leg]);
            cosmos_msg.toe_pos_y[leg] = m_to_int(msg->toe_position_measured_Y[leg]);
            cosmos_msg.toe_cmd_y[leg] = m_to_int(msg->toe_position_commanded_Y[leg]);
            cosmos_msg.toe_pos_z[leg] = m_to_int(msg->toe_position_measured_Z[leg]);
            cosmos_msg.toe_cmd_z[leg] = m_to_int(msg->toe_position_commanded_Z[leg]);
        }
        cosmos_msg.observed_period = (uint8_t) (msg->observed_period*1000);
        telem_publish(LEGSTAT, (char *)&cosmos_msg, sizeof(cosmos_msg));
    }
}

#endif // #ifdef TURRET
