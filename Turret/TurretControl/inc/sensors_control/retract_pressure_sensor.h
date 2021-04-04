#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int8_t is_retract_pressure_sensor_present();
const char* get_retract_pressure_sensor_device();

float get_retract_pressure();
float calculate_retract_pressure(int32_t raw_sensor_value);
