#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int8_t is_hammer_angle_sensor_present();
const char* get_hammer_angle_sensor_device();

float get_hammer_angle();
float get_hammer_velocity();
float calculate_hammer_angle(int32_t raw_sensor_value);
float calculate_hammer_velocity(float current_angle);
