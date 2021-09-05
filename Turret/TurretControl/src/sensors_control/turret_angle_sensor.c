#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "sclog4c/sclog4c.h"

#include "sensors_control/throw_pressure_sensor.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

static const int8_t k_is_turret_angle_sensor_present = 0;
static const char *k_turret_angle_sensor_device = "/sys/bus/iio/devices/iio:device0/in_voltage2_raw";

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static float s_turret_angle = 0.0f;
static float s_turret_velocity = 0.0f;

// -----------------------------------------------------------------------------
// public methods
// -----------------------------------------------------------------------------

int8_t is_turret_angle_sensor_present()
{
    return k_is_turret_angle_sensor_present;
}

const char* get_turret_angle_sensor_device()
{
    return k_turret_angle_sensor_device;
}

float get_turret_angle()
{
    return s_turret_angle;
}

float get_turret_velocity()
{
    return s_turret_velocity;
}

float calculate_turret_angle(int32_t raw_sensor_value)
{
    return (float)raw_sensor_value;
}

float calculate_turret_velocity(float current_angle)
{
    s_turret_velocity = current_angle;
    return s_turret_velocity;
}
