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
static stomp_tlm_cmd_hammer_conf_subscription_t *s_tml_cmd_hammer_conf_subscription;

// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

static void sensors_control_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_sensors_control *msg, void *user);
static void hammer_trigger_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_hammer_trigger *msg, void *user);
static void tlm_cmd_hammer_conf_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_tlm_cmd_hammer_conf *msg, void *user);

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

    // Convert all values from ARM representaion in float into format usable on PRU.  Jules and Pascals
    // can be used as is without worry of loosing fractional parts.  Radians get converted (and used) as 
    // milliradians.
    //
    // see: HammerControllerValues.md for definition of variable units and format
    //
    
    int32_t hammer_angle = (int32_t)(msg->hammer_angle * 1000.0f);
    int32_t hammer_velocity = (int32_t)(msg->hammer_velocity * 1000.0f);
    int32_t hammer_energy = (int32_t)msg->hammer_energy;
    int32_t available_break_energy = (int32_t)msg->available_break_energy;
    int32_t turret_angle = (int32_t)(msg->turret_angle * 1000.0f);
    int32_t turret_velocity = (int32_t)(msg->turret_velocity * 1000.0f);
    int32_t throw_pressure = (int32_t)msg->throw_pressure;
    int32_t retract_pressure = (int32_t)msg->retract_pressure;

    sprintf(sensors_message_buff, "SENS:HA:%d:HV:%d:HE:%d:BE:%d:TA:%d:TV:%d:TP:%d:RP:%d\n",
        hammer_angle,
        hammer_velocity,
        hammer_energy,
        available_break_energy,
        turret_angle,
        turret_velocity,
        throw_pressure,
        retract_pressure);

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

    char throw_message_buff[k_message_buff_len];

    // Convert all values from ARM representaion in float into format usable on PRU.  Jules and Pascals
    // can be used as is without worry of loosing fractional parts.  Radians get converted (and used) as 
    // milliradians.
    //
    // see: HammerControllerValues.md for definition of variable units and format
    //

    int32_t throw_intensity = (int32_t)(msg->throw_intensity * 1000.0f);
    int32_t retract_intensity = (int32_t)msg->retract_intensity;

    if (msg->trigger_type == STOMP_HAMMER_TRIGGER_THROW_RETRACT)
    {
        sprintf(throw_message_buff, "THRW:TYPE:THROW:TINTENSITY:%d:RINTENSITY:%d\n", throw_intensity, retract_intensity);
    }
    else if (msg->trigger_type == STOMP_HAMMER_TRIGGER_RETRACT_ONLY)
    {
        sprintf(throw_message_buff, "THRW:TYPE:RETRACT:RINTENSITY:%d\n", retract_intensity);
    }
    else
    {
        logm(SL4C_ERROR, "Invalid hammer triger type. %i", msg->trigger_type);
        return;
    }

    logm(SL4C_DEBUG, "SEND RPMSG: %s", throw_message_buff);

    if (write(g_rpmsg_fd, throw_message_buff, strlen(throw_message_buff)) < 0)
    {
        logm(SL4C_ERROR, "Could not send pru throw message. Error %i from write(): %s", errno, strerror(errno));
    }
}

// -----------------------------------------------------------------------------
// Stomp TLM_CMD Hammer Conf LCM Handlers
// -----------------------------------------------------------------------------

int tlm_cmd_hammer_conf_handler_init()
{
    s_tml_cmd_hammer_conf_subscription = stomp_tlm_cmd_hammer_conf_subscribe(g_lcm, TLM_CMD_HAMMER_CONF, &tlm_cmd_hammer_conf_handler, NULL);

    if (!s_tml_cmd_hammer_conf_subscription)
    {
        return -1;
    }

    return 0;
}

int tlm_cmd_hammer_conf_handler_shutdown()
{
    stomp_tlm_cmd_hammer_conf_unsubscribe(g_lcm, s_tml_cmd_hammer_conf_subscription);
    return 0;
}

static void tlm_cmd_hammer_conf_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_tlm_cmd_hammer_conf *msg, void *user)
{
    static struct timeval now;
    static struct timeval last_msg;
    gettimeofday(&now, 0);

    double microsec = ((now.tv_sec - last_msg.tv_sec) / 1000000.0f) + (now.tv_usec - last_msg.tv_usec);

    logm(SL4C_FINE, "%s msg received, dt = %f", channel, microsec);
    last_msg = now;

    // send the message down to the PRU

    char config_message_buff[k_message_buff_len];

    // Convert all values from ARM representaion in float into format usable on PRU.  Jules and Pascals
    // can be used as is without worry of loosing fractional parts.  Radians get converted (and used) as 
    // milliradians.
    //
    // see: HammerControllerValues.md for definition of variable units and format
    //

    int32_t max_throw_angle = (int32_t)(msg->max_throw_angle * 1000.0f);
    int32_t min_retract_angle = (int32_t)(msg->min_retract_angle * 1000.0f);
    int32_t break_exit_velocity = (int32_t)(msg->break_exit_velocity * 1000.0f);
    int32_t emergency_break_angle = (int32_t)(msg->emergency_break_angle * 1000.0f);
    int32_t valve_change_dt = (int32_t)msg->valve_change_dt;
    int32_t max_throw_pressure_dt = (int32_t)msg->max_throw_pressure_dt;
    int32_t max_throw_expand_dt = (int32_t)msg->max_throw_expand_dt;
    int32_t max_retract_pressure_dt = (int32_t)msg->max_retract_pressure_dt;
    int32_t max_retract_expand_dt = (int32_t)msg->max_retract_expand_dt;
    int32_t max_retract_break_dt = (int32_t)msg->max_retract_break_dt;
    int32_t max_retract_settle_dt = (int32_t)msg->max_retract_settle_dt;

    sprintf(config_message_buff, "CONF:TA:%d:RA:%d:BEV:%d:EBA:%d:VCDT:%d:TPDT:%d:TEDT:%d:RPDT:%d:REDT:%d:RBDT:%d:RSDT:%d\n",
        max_throw_angle,
        min_retract_angle,
        break_exit_velocity,
        emergency_break_angle,
        valve_change_dt,
        max_throw_pressure_dt,
        max_throw_expand_dt,
        max_retract_pressure_dt,
        max_retract_expand_dt,
        max_retract_break_dt,
        max_retract_settle_dt
    );

    logm(SL4C_DEBUG, "SEND RPMSG: %s", config_message_buff);

    if (strlen(config_message_buff) >= k_message_buff_len)
    {
        logm(SL4C_FATAL, "Message will not fit in maximum RPMsg message size");
        return;
    }

    if (write(g_rpmsg_fd, config_message_buff, strlen(config_message_buff)) < 0)
    {
        logm(SL4C_ERROR, "Could not send pru conf message. Error %i from write(): %s", errno, strerror(errno));
    }
}


