#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "sclog4c/sclog4c.h"

#include "sensors_control/throw_pressure_sensor.h"

// Throw Pressure Sensor is conned to BBAI P9.35

static const int8_t k_is_throw_pressure_sensor_present = 1;
static const char *k_throw_pressure_sensor_device = "/sys/bus/iio/devices/iio:device0/in_voltage4_raw";

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
    float throw_pressure = (float)raw_throw_pressure;

    logm(SL4C_DEBUG, "Throw Pressure = %f", throw_pressure);
    return throw_pressure;
}
