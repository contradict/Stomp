#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int32_t turret_angle_sensor_init(int8_t present, char* device, int32_t min_angle_deg, float min_angle_volts, int32_t max_angle_deg, float max_angle_volts, float dev_scale, float volt_scale);

int8_t is_turret_angle_sensor_present();
const char* get_turret_angle_sensor_device();

float get_turret_angle();
float get_turret_velocity();
void calculate_turret_values(int32_t raw_sensor_value);
