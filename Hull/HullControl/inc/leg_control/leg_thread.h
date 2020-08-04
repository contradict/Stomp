#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include <lcm/lcm.h>
#include <modbus.h>

#include "leg_control/queue.h"

struct leg_thread_state {
    volatile bool shouldrun;
    lcm_t *lcm;
    char *devname;
    uint32_t baud;
    uint32_t period;
    uint32_t response_timeout;
    modbus_t *ctx;
    struct queue parameter_queue;
    struct queue command_queue;
    struct queue response_queue;
    struct queue telemetry_queue;
};

struct leg_control_parameters {
    float forward_velocity;
    float angular_velocity;
};

pid_t create_leg_thread(struct leg_thread_state *leg_thread, const char * progname);
void terminate_leg_thread(struct leg_thread_state *leg_thread);
