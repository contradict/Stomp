#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


int8_t is_hammer_angle_sensor_present();
const char* get_hammer_angle_sensor_device();
float process_raw_hammer_angle(int32_t);
