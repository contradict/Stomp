#pragma once
#include <sys/types.h>
#include <stdbool.h>

#include <lcm/lcm.h>

#include "lcm/stomp_control_radio.h"
#include "lcm/stomp_turret_telemetry.h"

int control_radio_handler_init();
int control_radio_handler_shutdown();

void turret_telemetry_send(stomp_turret_telemetry *lcm_msg);

