#pragma once
#include <sys/types.h>
#include <stdbool.h>

#include <lcm/lcm.h>

#include "lcm/stomp_control_radio.h"

int control_radio_handler_init();
int control_radio_handler_shutdown();
