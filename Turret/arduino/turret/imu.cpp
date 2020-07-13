#include "I2C.h"
#include "MPU6050.h"
#include "imu.h"
#include "telemetryController.h"

static void saveIMUParameters(void);
static void restoreIMUParameters(void);

static MPU6050 s_IMUSerial;
static int16_t acceleration[3], angular_rate[3];
static int16_t temperature;
uint32_t last_imu_process;
bool stationary, imu_read_valid;
static enum Orientation best_orientation;
static int32_t sum_angular_rate;
static int16_t cross_norm;
static int16_t total_norm;

struct IMUParameters {
    int8_t dlpf_mode;
    uint32_t imu_period;
    int32_t stationary_threshold;
    int16_t upright_cross;
    int16_t min_valid_cross;
    int16_t max_valid_cross;
    int16_t max_total_norm;
    int16_t x_threshold;
    int16_t z_threshold;
} __attribute__((packed));

static struct IMUParameters params;

static struct IMUParameters EEMEM saved_params = {
    .dlpf_mode=MPU6050_DLPF_BW_20,
    .imu_period=100000,
    .stationary_threshold=200,
    .upright_cross = 410,
    .min_valid_cross = 820,
    .max_valid_cross = 2458,
    .max_total_norm = 3584,
    .x_threshold = 1229,
    .z_threshold = 2028
};


void initIMU(void) {
    restoreIMUParameters();
    I2c.begin();
    I2c.setSpeed(false);
    I2c.timeOut(2);
    //I2c.scan(telemetry_stream);
    s_IMUSerial.initialize();
    Telem.LogMessage(String("IMU.getDeviceID() = ") + s_IMUSerial.getDeviceID());
    s_IMUSerial.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
    s_IMUSerial.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
    s_IMUSerial.setDLPFMode(MPU6050_DLPF_BW_20);
    last_imu_process = micros();
}


static bool  maybeReadIMU(void)
{
    // if enough time has passed, read the IMU
    uint32_t now = micros();
    bool possibly_stationary = false;
    if(now-last_imu_process > params.imu_period) {
        last_imu_process = now;
        uint8_t imu_err = s_IMUSerial.getMotion6(
            &acceleration[0], &acceleration[1], &acceleration[2],
            &angular_rate[0], &angular_rate[1], &angular_rate[2]);
        if(imu_err != 0) {
            imu_read_valid = false;
            stationary = false;
            best_orientation = ORN_UNKNOWN;
            return false;
        }
        imu_read_valid = true;
        temperature = s_IMUSerial.getTemperature();
        sum_angular_rate = (abs(angular_rate[0]) +
                            abs(angular_rate[1]) +
                            abs(angular_rate[2]));
        total_norm = (int32_t)acceleration[0] * acceleration[0] / 2048 +
                     (int32_t)acceleration[1] * acceleration[1] / 2048 +
                     (int32_t)acceleration[2] * acceleration[2] / 2048;
        // still might have large acceleration and small angular rates
        possibly_stationary = (sum_angular_rate<params.stationary_threshold &&
                               total_norm < params.max_total_norm);
        // if possibly stationary, trigger a new orientation calculation
        // on the next call
        if(possibly_stationary) {
            best_orientation = ORN_UNKNOWN;
        } else {
            // if not stationary, refuse to guess
            best_orientation = ORN_UNKNOWN;
            stationary = false;
        }
    }
    return possibly_stationary;
}


// State machine to distribute compute over several cycles
void updateIMU(void) {
    if(maybeReadIMU()) {
        // Compute cross product with Zhat
        // a = measured
        // b = [0, 0, 1]
        // cx = ay*bz - az*by
        // cy = az*bx - ax*bz
        // cz = ax*by - ay*bx
        int32_t cross_x =  acceleration[1]; // * 1.0 in z
        int32_t cross_y = -acceleration[0];
        cross_norm = cross_x * cross_x / 2048 + cross_y * cross_y / 2048;
        if((cross_norm < params.upright_cross) &&
           (acceleration[2] > params.z_threshold))
        {
            best_orientation = ORN_UPRIGHT;
            stationary = true;
        } 
        else if((params.min_valid_cross < cross_norm) &&
                  (cross_norm<params.max_valid_cross)) 
        {
            if(abs(acceleration[0]) > params.x_threshold)
            {
                best_orientation = ORN_NOT_UPRIGHT;
                stationary = true;
            }
        } 
        else 
        {
            best_orientation = ORN_UNKNOWN;
            stationary = false;
        }
    }
}

void telemetryIMU(void) 
{
    Telem.SendIMUTelem(acceleration, angular_rate, temperature);
    Telem.SendORNTelem(stationary, best_orientation, sum_angular_rate, total_norm, cross_norm);
}


enum Orientation getOrientation(void) {
    return (enum Orientation)best_orientation;
}


bool getOmegaZ(int16_t *omega_z) {
    if(imu_read_valid) {
        *omega_z = angular_rate[2];
    }
    return imu_read_valid;
}

void setIMUParameters(
    int8_t dlpf, int32_t imu_period, int32_t stationary_threshold,
    int16_t upright_cross, int16_t min_valid_cross, int16_t max_valid_cross,
    int16_t max_total_norm, int16_t x_threshold, int16_t z_threshold)
{
    s_IMUSerial.setDLPFMode(dlpf);
    params.dlpf_mode = dlpf;
    params.imu_period = imu_period;
    params.stationary_threshold = stationary_threshold;
    params.upright_cross = upright_cross;
    params.min_valid_cross = min_valid_cross;
    params.max_valid_cross = max_valid_cross;
    params.max_total_norm = max_total_norm;
    params.x_threshold = x_threshold;
    params.z_threshold = z_threshold;
    saveIMUParameters();
}

void saveIMUParameters(void) {
    eeprom_write_block(&params, &saved_params, sizeof(struct IMUParameters));
}

void restoreIMUParameters(void) {
    eeprom_read_block(&params, &saved_params, sizeof(struct IMUParameters));
}

