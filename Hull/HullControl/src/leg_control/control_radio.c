#include <sys/types.h>

#include "channel_names.h"
#include "leg_control/control_radio.h"
#include "leg_control/leg_thread.h"

static
void control_radio_handler(const lcm_recv_buf_t *rbuf, const char *channel, const stomp_control_radio *msg, void *user)
{
    (void)rbuf;
    (void)channel;
    struct control_radio_state *state = user;
    ssize_t offset = ringbuf_acquire(state->parameter_queue.ringbuf, state->ringbuf_worker, sizeof(struct leg_control_parameters));
    if(offset > 0)
    {
        struct leg_control_parameters *params = (struct leg_control_parameters *)(state->parameter_queue.buffer + offset);
        params->forward_velocity = (msg->axis[0] - 1500.0f) / 500.0f;
        params->angular_velocity = (msg->axis[1] - 1500.0f) / 500.0f;
        ringbuf_produce(state->parameter_queue.ringbuf, state->ringbuf_worker);
    }
}

int control_radio_init(struct control_radio_state *state)
{
    state->ringbuf_worker = ringbuf_register(state->parameter_queue.ringbuf, 0);
    if(!state->ringbuf_worker)
        return -1;
    state->radio_subscription = stomp_control_radio_subscribe(state->lcm, SBUS_RADIO_COMMAND, &control_radio_handler, state);
    if(!state->radio_subscription)
        return -2;
    return 0;
}

int control_radio_shutdown(struct control_radio_state *state)
{
    ringbuf_unregister(state->parameter_queue.ringbuf, state->ringbuf_worker);
    stomp_control_radio_unsubscribe(state->lcm, state->radio_subscription);
    return 0;
}
