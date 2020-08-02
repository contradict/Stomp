#pragma once
#include <sys/types.h>
#include "leg_control/queue.h"

struct control_radio_thread_state {
    struct queue parameter_queue;
};

pid_t create_control_radio_thread(struct control_radio_thread_state *control_radio_thread_state);
