#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int8_t is_turret_angle_sensor_present();
const char* get_turret_angle_sensor_device();

float get_turret_angle();
float get_turret_velocity();
float calculate_turret_angle(int32_t raw_sensor_value);
float calculate_turret_velocity(float current_angle);
