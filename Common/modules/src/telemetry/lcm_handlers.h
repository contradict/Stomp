#pragma once

#include <lcm/lcm.h>
#include "lcm/stomp_control_radio.h"

#ifdef HULL
#include "lcm/stomp_telemetry_leg.h"
#endif

#ifdef TURRET
#include "lcm/stomp_turret_telemetry.h"
#include "lcm/stomp_sensors_control.h"
#endif

void sbus_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_control_radio *msg, void *user);

#ifdef HULL
void leg_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_telemetry_leg *msg, void *user);

#endif

#ifdef TURRET
void turret_telemetry_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_turret_telemetry *msg, void *user);

void turret_sensors_control_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_sensors_control *msg, void *user);
#endif
