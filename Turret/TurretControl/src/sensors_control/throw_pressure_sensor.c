#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "sclog4c/sclog4c.h"

#include "sensors_control/throw_pressure_sensor.h"

static const int8_t k_is_throw_pressure_sensor_present = 0;
static const char *k_throw_pressure_sensor_device = "/sys/bus/iio/devices/iio:device0/in_voltage3_raw";

int8_t is_throw_pressure_sensor_present()
{
    return k_is_throw_pressure_sensor_present;
}

const char* get_throw_pressure_sensor_device()
{
    return k_throw_pressure_sensor_device;
}

float process_raw_throw_pressure(int32_t raw_throw_pressure)
{
    return (float)raw_throw_pressure;
}
