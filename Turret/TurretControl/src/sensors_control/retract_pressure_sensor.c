#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "sclog4c/sclog4c.h"

#include "sensors_control/retract_pressure_sensor.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

static const int8_t k_is_retract_pressure_sensor_present = 1;
static const char *k_retract_pressure_sensor_device = "/sys/bus/iio/devices/iio:device0/in_voltage6_raw";

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static float s_retract_pressure = 0.0f;

// -----------------------------------------------------------------------------
// public methods
// -----------------------------------------------------------------------------

int8_t is_retract_pressure_sensor_present()
{
    return k_is_retract_pressure_sensor_present;
}

const char* get_retract_pressure_sensor_device()
{
    return k_retract_pressure_sensor_device;
}

float get_retract_pressure()
{
    return s_retract_pressure;
}

float calculate_retract_pressure(int32_t raw_sensor_value)
{
    s_retract_pressure = (float)raw_sensor_value;

    logm(SL4C_DEBUG, "Retract Pressure = %f", s_retract_pressure);
    return s_retract_pressure;
}
