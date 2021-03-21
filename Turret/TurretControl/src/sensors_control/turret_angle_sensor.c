#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "sclog4c/sclog4c.h"

#include "sensors_control/throw_pressure_sensor.h"

static const int8_t k_is_turret_angle_sensor_present = 0;
static const char *k_turret_angle_sensor_device = "/sys/bus/iio/devices/iio:device0/in_voltage2_raw";

int8_t is_turret_angle_sensor_present()
{
    return k_is_turret_angle_sensor_present;
}

const char* get_turret_angle_sensor_device()
{
    return k_turret_angle_sensor_device;
}

float process_raw_turret_angle(int32_t raw_turret_angle)
{
    return (float)raw_turret_angle;
}
