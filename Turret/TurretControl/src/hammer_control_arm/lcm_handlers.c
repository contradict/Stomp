#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include "sclog4c/sclog4c.h"

#include "sbus_channels.h"
#include "hammer_control_arm/hammer_control_arm.h"
#include "hammer_control_arm/lcm_handlers.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

static const uint32_t k_message_buff_len = 512;

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static stomp_sensors_control_subscription_t *s_sensors_control_subscription;
static stomp_hammer_trigger_subscription_t *s_hammer_trigger_subscription;

// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

static void sensors_control_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_sensors_control *msg, void *user);
static void hammer_trigger_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_hammer_trigger *msg, void *user);

// -----------------------------------------------------------------------------
// Stomp Sensor Control LCM Handlers
// -----------------------------------------------------------------------------

int sensors_control_handler_init()
{
    s_sensors_control_subscription = stomp_sensors_control_subscribe(g_lcm, SENSORS_CONTROL, &sensors_control_handler, NULL);

    if (!s_sensors_control_subscription)
    {
        return -1;
    }

    return 0;
}

int sensors_control_handler_shutdown()
{
    stomp_sensors_control_unsubscribe(g_lcm, s_sensors_control_subscription);
    return 0;
}

static void sensors_control_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_sensors_control *msg, void *user)
{
    static struct timeval now;
    static struct timeval last_msg;
    gettimeofday(&now, 0);

    double microsec = ((now.tv_sec - last_msg.tv_sec) / 1000000.0f) + (now.tv_usec - last_msg.tv_usec);

    logm(SL4C_FINE, "%s msg received, dt = %f", channel, microsec);
    last_msg = now;

    // send the message down to the PRU

    char sensors_message_buff[k_message_buff_len];

    sprintf(sensors_message_buff, "SENS:HA:%d:%d:TA:%d:%d:TP:%d:RP:%d\n",
        (int32_t)msg->hammer_angle,
        (int32_t)msg->hammer_velocity,
        (int32_t)msg->turret_angle,
        (int32_t)msg->turret_velocity,
        (int32_t)msg->throw_pressure,
        (int32_t)msg->retract_pressure);

    logm(SL4C_DEBUG, "SEND RPMSG: %s", sensors_message_buff);

    if (strlen(sensors_message_buff) >= k_message_buff_len)
    {
        logm(SL4C_FATAL, "Message will not fit in maximum RPMsg message size");
        return;
    }

    if (write(g_rpmsg_fd, sensors_message_buff, strlen(sensors_message_buff)) < 0)
    {
        logm(SL4C_ERROR, "Could not send pru sens message. Error %i from write(): %s", errno, strerror(errno));
    }
}

// -----------------------------------------------------------------------------
// Stomp Hammer Trigger LCM Handlers
// -----------------------------------------------------------------------------

int hammer_trigger_handler_init()
{
    s_hammer_trigger_subscription = stomp_hammer_trigger_subscribe(g_lcm, HAMMER_TRIGGER, &hammer_trigger_handler, NULL);

    if (!s_hammer_trigger_subscription)
    {
        return -1;
    }

    return 0;
}

int hammer_trigger_handler_shutdown()
{
    stomp_hammer_trigger_unsubscribe(g_lcm, s_hammer_trigger_subscription);
    return 0;
}

static void hammer_trigger_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_hammer_trigger *msg, void *user)
{
    static struct timeval now;
    static struct timeval last_msg;
    gettimeofday(&now, 0);

    double microsec = ((now.tv_sec - last_msg.tv_sec) / 1000000.0f) + (now.tv_usec - last_msg.tv_usec);

    logm(SL4C_FINE, "%s msg received, dt = %f", channel, microsec);
    last_msg = now;

    // send the message down to the PRU

    char fire_message_buff[k_message_buff_len];

    sprintf(fire_message_buff, "FIRE:ANGLE:20");

    logm(SL4C_DEBUG, "SEND RPMSG: %s", fire_message_buff);

    if (write(g_rpmsg_fd, fire_message_buff, strlen(fire_message_buff)) < 0)
    {
        logm(SL4C_ERROR, "Could not send pru fire message. Error %i from write(): %s", errno, strerror(errno));
    }
}


