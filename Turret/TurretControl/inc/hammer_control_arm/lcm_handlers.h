#pragma once
#include <sys/types.h>
#include <stdbool.h>

#include <lcm/lcm.h>

#include "lcm/stomp_sensors_control.h"
#include "lcm/stomp_hammer_trigger.h"

int sensors_control_handler_init();
int sensors_control_handler_shutdown();

int hammer_trigger_handler_init();
int hammer_trigger_handler_shutdown();

