#include <sys/types.h>

#include "sclog4c/sclog4c.h"

#include "lcm_channels.h"
#include "sbus_channels.h"
#include "turret_control/turret_control.h"
#include "turret_control/lcm_handlers.h"

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static stomp_control_radio_subscription_t * s_control_radio_subscription;

// -----------------------------------------------------------------------------
// Stomp Control Radio LCM Handlers
// -----------------------------------------------------------------------------

static void control_radio_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_control_radio *msg, void *user)
{
    (void)rbuf;
    (void)channel;

    /*
    switch(msg->toggle[HULL_MOTION_SEL])
    {
        case STOMP_CONTROL_RADIO_OFF:
            params->motion = MOTION_MODE_STEERING;
            params->angular_velocity = msg->axis[HULL_OMEGA_Z];
            break;
        case STOMP_CONTROL_RADIO_CENTER:
            params->motion = MOTION_MODE_TRANSLATING;
            params->right_velocity = msg->axis[HULL_VELOCITY_Y];
            break;
        case STOMP_CONTROL_RADIO_ON:
            params->motion = MOTION_MODE_DRIVING;
            params->right_velocity = msg->axis[HULL_VELOCITY_Y];
            params->angular_velocity = msg->axis[HULL_OMEGA_Z_D];
            break;
    }

    params->forward_velocity = msg->axis[HULL_VELOCITY_X];
    params->ride_height = msg->axis[HULL_RIDE_HEIGHT];
    params->left = msg->axis[HULL_LS];
    params->right = msg->axis[HULL_RS];

    switch(msg->toggle[HULL_MODE])
    {
        case STOMP_CONTROL_RADIO_ON:
            params->lock = LOCK_LOCK;
            break;
        case STOMP_CONTROL_RADIO_OFF:
            params->lock = LOCK_FREE;
            break;
    }

    switch(msg->toggle[HULL_ENABLE])
    {
        case STOMP_CONTROL_RADIO_ON:
            params->enable = ENABLE_WALK;
            break;
        case STOMP_CONTROL_RADIO_OFF:
            params->enable = ENABLE_DISABLE;
            break;
    }

    switch(msg->toggle[HULL_GAIT])
    {
        case STOMP_CONTROL_RADIO_ON:
            params->gait_selection = 2;
            break;
        case STOMP_CONTROL_RADIO_CENTER:
            params->gait_selection = 1;
            break;
        case STOMP_CONTROL_RADIO_OFF:
            params->gait_selection = 0;
            break;
    }

    if(msg->failsafe || msg->no_data)
    {
        params->enable = ENABLE_DISABLE;
        params->lock = LOCK_FREE;
    }
    */
}

int control_radio_init()
{
    s_control_radio_subscription = stomp_control_radio_subscribe(g_lcm, SBUS_RADIO_COMMAND, &control_radio_handler, NULL);

    if (!s_control_radio_subscription)
    {
        return -1;
    }

    return 0;
}

int control_radio_shutdown()
{
    stomp_control_radio_unsubscribe(g_lcm, s_control_radio_subscription);
    return 0;
}

