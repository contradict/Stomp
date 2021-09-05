#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int8_t is_throw_pressure_sensor_present();
const char* get_throw_pressure_sensor_device();

float get_throw_pressure();
float calculate_throw_pressure(int32_t raw_sensor_value);
