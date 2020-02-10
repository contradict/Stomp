#ifndef SENSORS_H
#define SENSORS_H

void sensorSetup();
bool readMlhPressure(int16_t* pressure);
bool readAngle(uint16_t* angle);
bool angularVelocity(float* angular_velocity);
bool angularVelocityBuffered(float* angular_velocity, const uint16_t* angle_data, uint16_t datapoints_buffered, uint16_t timestep_ms);
void readImu(float* our_forward_vel, float* our_angular_vel);
void resetImu();
bool readSensors(void);
uint16_t getAngle(void);
int16_t getPressure(void);

#endif // SENSORS_H
