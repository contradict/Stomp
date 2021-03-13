#include <sys/types.h>

#include "sclog4c/sclog4c.h"

#include "lcm_channels.h"
#include "lcm_channels.h"
#include "sbus_channels.h"
#include "leg_control/lcm_handlers.h"
#include "leg_control/leg_thread.h"
#include "lcm/stomp_telemetry_leg.h"
#include "lcm/stomp_modbus.h"

static
void control_radio_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_control_radio *msg, void *user)
{
    (void)rbuf;
    (void)channel;
    struct lcm_listener_state *state = user;
    ssize_t offset = ringbuf_acquire(state->queue->ringbuf, state->worker, sizeof(struct leg_control_parameters));
    if(offset >= 0)
    {
        struct leg_control_parameters *params = (struct leg_control_parameters *)(state->queue->buffer + offset);
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
        ringbuf_produce(state->queue->ringbuf, state->worker);
    }
}

int control_radio_init(struct lcm_listener_state *state)
{
    state->worker = ringbuf_register(state->queue->ringbuf, 0);
    if(!state->worker)
        return -1;
    state->subscription = stomp_control_radio_subscribe(state->lcm, SBUS_RADIO_COMMAND, &control_radio_handler, state);
    if(!state->subscription)
        return -2;
    return 0;
}

int control_radio_shutdown(struct lcm_listener_state *state)
{
    ringbuf_unregister(state->queue->ringbuf, state->worker);
    stomp_control_radio_unsubscribe(state->lcm, (stomp_control_radio_subscription_t *)state->subscription);
    return 0;
}

int lcm_telemetry_send(struct lcm_sender_state* state)
{
    size_t offset;
    size_t s = ringbuf_consume(state->queue->ringbuf, &offset);
    if(s >= sizeof(stomp_telemetry_leg))
    {
        size_t rs = s;
        while(s >= sizeof(stomp_telemetry_leg))
        {
            stomp_telemetry_leg *telem = (stomp_telemetry_leg*)(state->queue->buffer + offset);
            stomp_telemetry_leg_publish(state->lcm, LEG_TELEMETRY, telem);
            s -= sizeof(stomp_telemetry_leg);
            offset += sizeof(stomp_telemetry_leg);
        }
        ringbuf_release(state->queue->ringbuf, rs);
    }
    return s;
}

int lcm_response_send(struct lcm_sender_state* state)
{
    size_t offset;
    size_t s = ringbuf_consume(state->queue->ringbuf, &offset);
    if(s >= sizeof(stomp_modbus))
    {
        size_t rs = s;
        while(s >= sizeof(stomp_modbus))
        {
            stomp_modbus *mb = (stomp_modbus*)(state->queue->buffer + offset);
            stomp_modbus_publish(state->lcm, LEG_RESPONSE, mb);
            s -= sizeof(stomp_telemetry_leg);
            offset += sizeof(stomp_telemetry_leg);
        }
        ringbuf_release(state->queue->ringbuf, rs);
    }
    return s;
}


static
void modbus_command_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_modbus *msg, void *user)
{
    (void)rbuf;
    (void)channel;
    struct lcm_listener_state *state = user;
    ssize_t offset = ringbuf_acquire(state->queue->ringbuf, state->worker, sizeof(struct leg_control_parameters));
    if(offset >= 0)
    {
        stomp_modbus *modbus = (stomp_modbus *)(state->queue->buffer + offset);
        memcpy(modbus, msg, sizeof(stomp_modbus));
        ringbuf_produce(state->queue->ringbuf, state->worker);
    }
}

int modbus_command_init(struct lcm_listener_state *state)
{
    state->worker = ringbuf_register(state->queue->ringbuf, 0);
    if(!state->worker)
        return -1;
    state->subscription = stomp_modbus_subscribe(state->lcm, LEG_COMMAND, &modbus_command_handler, state);
    if(!state->subscription)
        return -2;
    return 0;
}

int modbus_command_shutdown(struct lcm_listener_state *state)
{
    ringbuf_unregister(state->queue->ringbuf, state->worker);
    stomp_modbus_unsubscribe(state->lcm, (stomp_modbus_subscription_t *)state->subscription);
    return 0;
}
