#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include <lcm/lcm.h>
#include <modbus.h>
#include <toml.h>

#include "leg_control/queue.h"

struct leg_thread_definition {
    lcm_t *lcm;
    char *devname;
    uint32_t baud;
    float frequency;
    uint32_t byte_timeout;
    uint32_t response_timeout;
    toml_table_t *config;
    struct queue *parameter_queue;
    struct queue *command_queue;
    struct queue *response_queue;
    struct queue *telemetry_queue;
};

enum leg_control_enable {
    ENABLE_WALK,
    ENABLE_DISABLE,
};

enum leg_control_lock {
    LOCK_FREE,
    LOCK_LOCK,
};

struct leg_control_parameters {
    float forward_velocity;
    float angular_velocity;
    enum leg_control_enable enable;
    enum leg_control_lock lock;
    int gait_selection;
};

struct leg_thread_state* create_leg_thread(struct leg_thread_definition *leg_thread, const char * progname);
void terminate_leg_thread(struct leg_thread_state **leg_thread);
