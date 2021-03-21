#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "sclog4c/sclog4c.h"

#include "sensors_control/hammer_angle_sensor.h"

static const int8_t k_is_hammer_angle_sensor_present = 1;
static const char *k_hammer_angle_sensor_device = "/sys/bus/iio/devices/iio:device0/in_voltage7_raw";

int8_t is_hammer_angle_sensor_present()
{
    return k_is_hammer_angle_sensor_present;
}

const char* get_hammer_angle_sensor_device()
{
    return k_hammer_angle_sensor_device;
}

float process_raw_hammer_angle(int32_t raw_hammer_angle)
{
    return (float)raw_hammer_angle;
}
