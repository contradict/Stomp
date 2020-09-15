#pragma once
#include <sys/types.h>
#include <stdbool.h>

#include <lcm/lcm.h>

#include <ringbuf.h>

#include "leg_control/queue.h"
#include "lcm/stomp_control_radio.h"

struct lcm_listener_state {
    lcm_t *lcm;
    struct queue *queue;
    ringbuf_worker_t *worker;
    void *subscription;
};

struct lcm_sender_state {
    lcm_t *lcm;
    struct queue *queue;
};


int control_radio_init(struct lcm_listener_state *state);
int control_radio_shutdown(struct lcm_listener_state *state);
int lcm_telemetry_send(struct lcm_sender_state* state);
int lcm_response_send(struct lcm_sender_state* state);
int modbus_command_init(struct lcm_listener_state *state);
int modbus_command_shutdown(struct lcm_listener_state *state);
