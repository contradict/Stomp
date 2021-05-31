#pragma once
#include <sys/types.h>
#include <stdbool.h>

#include <lcm/lcm.h>

#include "lcm/stomp_control_radio.h"

int control_radio_init();
int control_radio_shutdown();

// defined in Common/inc/lcm/lcm_channels.h

extern const char* SBUS_RADIO_COMMAND;
extern const char* SENSORS_CONTROL;
extern const char* TURRET_TELEMETRY;
extern const char* HAMMER_CONFIG;

/*
int lcm_telemetry_send(struct lcm_sender_state* state);
int lcm_response_send(struct lcm_sender_state* state);
int modbus_command_init(struct lcm_listener_state *state);
int modbus_command_shutdown(struct lcm_listener_state *state);
*/
