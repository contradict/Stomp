#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


int8_t is_turret_angle_sensor_present();
const char* get_turret_angle_sensor_device();
float process_raw_turret_angle(int32_t);
