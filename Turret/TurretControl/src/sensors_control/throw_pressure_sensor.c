#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>

#include "sclog4c/sclog4c.h"
#include "sensors_control/throw_pressure_sensor.h"

// Throw Pressure Sensor is connected to BBAI P9.3? (Chomp BeagleCape A3)

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

#define k_max_device_len 512

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static int8_t s_is_throw_pressure_sensor_present;
static char s_throw_pressure_sensor_device[k_max_device_len];

static float s_min_pressure_pascals;
static int s_min_pressure_count;
static float s_max_pressure_pascals;
static int s_max_pressure_count;

static float s_throw_pressure = 0.0f;

// -----------------------------------------------------------------------------
// public methods
// -----------------------------------------------------------------------------

int32_t throw_pressure_sensor_init(int8_t present, char* device, int32_t min_pressure_psi, float min_pressure_volts, int32_t max_pressure_psi, float max_pressure_volts, float dev_scale, float volt_scale)
{
    s_is_throw_pressure_sensor_present = present;

    logm(SL4C_DEBUG, "Initialize Throw Pressure Sensor: Is Present %d", s_is_throw_pressure_sensor_present);

    if (strlen(device) >= k_max_device_len)
    {
        logm(SL4C_FATAL, "Device String too long");
        return -1;
    }

    strcpy (s_throw_pressure_sensor_device, device);

    // Conver degrees to radian

    s_min_pressure_pascals = min_pressure_psi * 6895.0f;
    s_max_pressure_pascals = max_pressure_psi * 6895.0f;
    
    // convert from the actual sensor voltage, down to what the cape analog opamp outputs (volt_scale)
    // which is in Amps.  Then convert to milliamps (A * 1000, because that is what the dev_scale
    // expects) then from voltage to count, scale provided by hardware

    s_min_pressure_count = (int32_t)((min_pressure_volts * volt_scale * 1000.0f) / dev_scale);
    s_max_pressure_count = (int32_t)((max_pressure_volts * volt_scale * 1000.0f) / dev_scale);

    return 0;
}
int8_t is_throw_pressure_sensor_present()
{
    return s_is_throw_pressure_sensor_present;
}

const char* get_throw_pressure_sensor_device()
{
    return s_throw_pressure_sensor_device;
}

float get_throw_pressure()
{
    return s_throw_pressure;
}

// MJS TODO: move to utils

static float lerp(float v0, float v1, float t) 
{
  return (1.0f - t) * v0 + t * v1;
}

void calculate_throw_values(int32_t raw_sensor_value)
{
    logm(SL4C_DEBUG, "Raw Angle Sensor Value = %d", raw_sensor_value);

    // map from count to pressure
    
    float t = ((float)raw_sensor_value - s_min_pressure_count) /
        (s_max_pressure_count - s_min_pressure_count);

    s_throw_pressure = lerp(s_min_pressure_pascals, s_max_pressure_pascals, t);
}
