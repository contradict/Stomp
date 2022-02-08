#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int32_t throw_pressure_sensor_init(int8_t present, char* device, int32_t min_pressure_psi, float min_pressure_volts, int32_t max_pressure_psi, float max_pressure_volts, float dev_scale, float volt_scale);

int8_t is_throw_pressure_sensor_present();
const char* get_throw_pressure_sensor_device();

float get_throw_pressure();
void calculate_throw_values(int32_t raw_sensor_value);
