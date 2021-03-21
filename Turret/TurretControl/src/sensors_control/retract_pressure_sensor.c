#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "sclog4c/sclog4c.h"

#include "sensors_control/retract_pressure_sensor.h"

static const int8_t k_is_retract_pressure_sensor_present = 1;
static const char *k_retract_pressure_sensor_device = "/sys/bus/iio/devices/iio:device0/in_voltage6_raw";

int8_t is_retract_pressure_sensor_present()
{
    return k_is_retract_pressure_sensor_present;
}

const char* get_retract_pressure_sensor_device()
{
    return k_retract_pressure_sensor_device;
}

float process_raw_retract_pressure(int32_t raw_retract_pressure)
{
    float retract_pressure = (float)raw_retract_pressure;

    logm(SL4C_DEBUG, "Retract Pressure = %f", retract_pressure);
    return retract_pressure;
}
