#include <sys/types.h>

#include "sclog4c/sclog4c.h"

#include "lcm_channels.h"
#include "sbus_channels.h"
#include "turret_control/turret_control.h"
#include "turret_control/lcm_handlers.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

static const float k_hammer_throw_threshold = 0.8f;
static const float k_hammer_retract_threshold = -0.8f;

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static stomp_control_radio_subscription_t * s_control_radio_subscription;

// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

static void control_radio_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_control_radio *msg, void *user);

// -----------------------------------------------------------------------------
// Stomp Control Radio LCM Handlers
// -----------------------------------------------------------------------------

int control_radio_init()
{
    s_control_radio_subscription = stomp_control_radio_subscribe(g_lcm, SBUS_RADIO_COMMAND, &control_radio_handler, NULL);

    if (!s_control_radio_subscription)
    {
        return -1;
    }

    return 0;
}

static void control_radio_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_control_radio *msg, void *user)
{
    (void)rbuf;
    (void)channel;


    //  Invalid or empty message go to safe mode
    
    if(msg->failsafe || msg->no_data)
    {
        logm(SL4C_FINE, "Received Invalid Radio Message\n");

        g_radio_control_parameters.enable = TURRET_DISABLED;
        g_radio_control_parameters.hammer_trigger = HAMMER_SAFE;
        return;
    }

    logm(SL4C_FINE, "Received Valid Radio Message\n");

    //  Grab the intensities from the axis
    
    g_radio_control_parameters.rotation_intensity = msg->axis[TURRET_ROTATION_INTENSITY];
    g_radio_control_parameters.throw_intensity = (msg->axis[TURRET_THROW_INTENSITY] + 1.0f) / 2.0f;
    g_radio_control_parameters.retract_intensity = (msg->axis[TURRET_RETRACT_INTENSITY] + 1.0f) / 2.0f;

    //  Turret Enable
    
    switch (msg->toggle[TURRET_ENABLE])
    {
        case STOMP_CONTROL_RADIO_ON:
            g_radio_control_parameters.enable = TURRET_ENABLED;
            break;

        default:
            g_radio_control_parameters.enable = TURRET_DISABLED;
            break;
    }

    //  Throw or Retract the hammer.  A bit strange because we are using an axis rather
    //  than a toggle for throw / safe / retract
    
    if (msg->axis[TURRET_HAMMER_TRIGGER] >= k_hammer_throw_threshold)
    {
        g_radio_control_parameters.hammer_trigger = HAMMER_TRIGGER_THROW;
    }
    else if (msg->axis[TURRET_HAMMER_TRIGGER] <= k_hammer_retract_threshold)
    {
        g_radio_control_parameters.hammer_trigger = HAMMER_TRIGGER_RETRACT;
    }
    else
    {
        g_radio_control_parameters.hammer_trigger = HAMMER_SAFE;
    }

    // Turret rotation mode 

    switch(msg->toggle[TURRET_ROTATION_MODE])
    {
        case STOMP_CONTROL_RADIO_OFF:
            g_radio_control_parameters.rotation_mode = ROTATION_MODE_DISABLED;
            break;

        case STOMP_CONTROL_RADIO_CENTER:
            g_radio_control_parameters.rotation_mode = ROTATION_MODE_MANUAL;
            break;

        case STOMP_CONTROL_RADIO_ON:
            g_radio_control_parameters.rotation_mode = ROTATION_MODE_AUTO;
            break;
    }
}

int control_radio_shutdown()
{
    stomp_control_radio_unsubscribe(g_lcm, s_control_radio_subscription);
    return 0;
}

