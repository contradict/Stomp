#include <inttypes.h>
#include <unistd.h>
#include <lcm/lcm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>

#include "messages.h"
#include "telemetry.h"
#include "sbus_channels.h"
#include "sclog4c/sclog4c.h"
#include "utils/utils.h"

#include "lcm_handlers.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

const int sbus_period_msec = 100;  // control radio packet send period

// -----------------------------------------------------------------------------
// lcm_message_handlers
// -----------------------------------------------------------------------------


void sbus_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_control_radio *msg, void *user)
{
    static struct timeval now;
    static struct timeval last_send_time;

    gettimeofday(&now, 0);
    int millis = time_diff_msec(last_send_time, now);
    logm(SL4C_DEBUG, "%d msec since last %s msg received", millis, channel);

    if (millis > sbus_period_msec)
    {
        gettimeofday(&last_send_time, 0);
        logm(SL4C_DEBUG, "Sending COSMOS SBUS Contol Radio packet");

        struct control_radio_cosmos cosmos_msg;

        memcpy(cosmos_msg.axis, msg->axis, STOMP_CONTROL_RADIO_AXES * sizeof(float));
        memcpy(cosmos_msg.toggle, msg->toggle, STOMP_CONTROL_RADIO_TOGGLES * sizeof(int8_t));
        cosmos_msg.failsafe = msg->failsafe;
        cosmos_msg.no_data = msg->no_data;

        telem_publish(CONTROL_RADIO, (char *)&cosmos_msg, sizeof(cosmos_msg));
    }
}
