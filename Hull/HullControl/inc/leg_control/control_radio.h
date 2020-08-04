#pragma once
#include <sys/types.h>
#include <stdbool.h>

#include <lcm/lcm.h>

#include "leg_control/queue.h"
#include "lcm/stomp_control_radio.h"

struct control_radio_state {
    lcm_t *lcm;
    struct queue parameter_queue;
    ringbuf_worker_t *ringbuf_worker;
    stomp_control_radio_subscription_t *radio_subscription;
};

int control_radio_init(struct control_radio_state *state);
int control_radio_shutdown(struct control_radio_state *state);
