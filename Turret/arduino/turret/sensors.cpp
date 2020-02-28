#include "Arduino.h"
#include "sensors.h"
#include "pins.h"

//  BB MJS: Removed VACUUM LEFT AND VACUUM_AI_RIGHT

static uint16_t cached_angle;
static int16_t cached_pressure;

void initSensors(){
    pinMode(ANGLE_AI, INPUT);
    pinMode(PRESSURE_AI, INPUT);
}

// static const uint32_t pressure_sensor_range = 920 - 102;
bool readMlhPressure(int16_t* pressure){
    uint16_t counts = analogRead(PRESSURE_AI);
    if (counts < 102) {
        *pressure = 0;
        return true;
    }
    if (counts > 920) {
        *pressure = 500;
        return true;
    }
    // safe with 16 bit uints, accurate to 0.02%%
    *pressure = (int16_t) (counts - 102) * 11 / 18;
    return true;
}

#define MIN_ANGLE_ANALOG_READ 50
#define ZERO_ANGLE_ANALOG_READ 102
#define MAX_ANGLE_ANALOG_READ 920
// 0 deg is 10% of input voltage, empirically observed to be 100 counts
// 360 deg is 90% of input voltage, empirically observed to be 920 counts
bool readAngle(uint16_t* angle){
    uint16_t counts = analogRead(ANGLE_AI);
    if ( counts < MIN_ANGLE_ANALOG_READ ) {
        // Failure mode in shock, rails to 0;
        return false;
    }
    if ( counts < ZERO_ANGLE_ANALOG_READ ) {
        *angle = 0;
        return true;
    }
    if ( counts > MAX_ANGLE_ANALOG_READ ) {
        *angle = 359;
        return true;
    }

    // safe with 16 bit uints, accurate to 0.02%
    *angle = (int16_t) (counts - ZERO_ANGLE_ANALOG_READ) * 11 / 25;
    return true;
}

uint16_t getAngle(void) {
    return cached_angle;
}

int16_t getPressure(void) {
    return cached_pressure;
}

bool readSensors(void) {
    return readAngle(&cached_angle) &&
           readMlhPressure(&cached_pressure);
}

#define ANGLE_CONVERSION_FLOAT 0.4400978f  // 360 / (920 - 102)
bool readAngleFloat(float* angle){
    uint16_t counts = analogRead(ANGLE_AI);
    if ( counts < MIN_ANGLE_ANALOG_READ ) { return false; } // Failure mode in shock, rails to 0;
    if ( counts < ZERO_ANGLE_ANALOG_READ ) { *angle = 0.0; return true; }
    if ( counts > MAX_ANGLE_ANALOG_READ ) { *angle = 359.9; return true; }

    *angle = (float) (counts - ZERO_ANGLE_ANALOG_READ) * ANGLE_CONVERSION_FLOAT;
    return true;
}

bool angularVelocity (float* angular_velocity) {
    // This function could filter low angle values and ignore them for summing.
    // If we only rail to 0V, we could still get a velocity.
    float angle_traversed = 0;
    float abs_angle_traversed = 0;
    float last_angle;
    bool angle_read_ok = readAngleFloat(&last_angle);
    float new_angle;
    float delta;
    uint32_t read_time = micros();
    // Take 50 readings. This should be ~25 ms. 1 rps = 2.78 deg/s
    uint8_t num_readings = 50;
    for (uint8_t i = 0; i < num_readings; i++) {
        if (readAngleFloat(&new_angle)) {
            delta = new_angle - last_angle;
            abs_angle_traversed += abs(delta);
            angle_traversed += delta;
            last_angle = new_angle;
            delayMicroseconds(300);
        } else {
            angle_read_ok = false;
        }
    }
    // if angle read ever sketchy or if data too noisy, do not return angular velocity
    if (angle_read_ok && abs(angle_traversed) - angle_traversed < num_readings * 2) {
        float read_time_seconds = (float) (micros() - read_time) / 1000000.0f;    // convert to milliseconds
        *angular_velocity = angle_traversed / read_time_seconds;  // degrees per second
        return true;
    } else {
        return false;
    }
}

// Returns absolute angular velocity in deg/sec
bool angularVelocityBuffered (float* angular_velocity, const uint16_t* angle_data, uint16_t datapoints_buffered, uint16_t timestep_ms ) {
    const uint32_t DATAPOINTS_TO_AVERAGE = 20;
    // do not report velocity if too few datapoints have been buffered
    if (datapoints_buffered < DATAPOINTS_TO_AVERAGE) {
        return false;
    }
    
    uint32_t angle_min = 360;
    uint32_t angle_max = 0;
    for (uint16_t i = datapoints_buffered - DATAPOINTS_TO_AVERAGE; i < datapoints_buffered; i++) {
        if (angle_data[i] < angle_min) { 
           angle_min = angle_data[i];
         }
        if (angle_data[i] > angle_max) { 
            angle_max = angle_data[i];
          }
    }

    *angular_velocity = (angle_max - angle_min) * 1000 / (DATAPOINTS_TO_AVERAGE * timestep_ms);
    return true;
}
