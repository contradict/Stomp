#ifndef TURRET_IMU_H
#define TURRET_IMU_H

enum Orientation {
    ORN_UPRIGHT = 0,
    ORN_NOT_UPRIGHT = 1,
    ORN_UNKNOWN = 2
};

void initIMU(void);
void updateIMU(void);
void telemetryIMU(void);
bool isStationary(void);
bool getOmegaZ(int16_t *omega_z);
enum Orientation getOrientation(void);
void setIMUParameters(
    int8_t dlpf, int32_t imu_period, int32_t stationary_threshold,
    int16_t upright_cross, int16_t min_valid_cross, int16_t max_valid_cross,
    int16_t max_total_norm, int16_t x_threshold, int16_t z_threshold);

#endif // TURRET_IMU_H
