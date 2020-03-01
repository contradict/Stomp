#include <Arduino.h>
#include "autofire.h"
#include "imu.h"
#include "telem.h"

extern uint8_t HAMMER_INTENSITIES_ANGLE[9];

struct AutoFireParameters {
    int32_t xtol, ytol;
    int32_t max_omegaZ;   // rad/s * 2048 = 50 deg/sec
    uint32_t autofire_telem_interval;
} __attribute__((packed));

static struct AutoFireParameters params;
static uint32_t last_autofire_telem = 0;

static struct AutoFireParameters EEMEM saved_params = {
    .xtol = 200,   // currently unused, front of box is depth
    .ytol = 200,
    .max_omegaZ = 1787,   // rad/s * 2048 = 50 deg/sec
    .autofire_telem_interval = 100000,
};

static void saveAutoFireParameters(void);

//  BB MJS: TMP

int32_t getTmpAdjustment()
{
    return params.max_omegaZ;
}

//  BB MJS: END TMP

int32_t swingDuration(int16_t hammer_intensity) {
    int16_t hammer_angle = HAMMER_INTENSITIES_ANGLE[hammer_intensity];
    int32_t x=(40L-hammer_angle);
    return 230L + (3L*x*x*x)/1024L;
}

bool omegaZLockout(int32_t *omegaZ) {
    *omegaZ = 0;
    int16_t rawOmegaZ;
    bool imu_valid = getOmegaZ(&rawOmegaZ);
    *omegaZ = (int32_t)rawOmegaZ*35L/16L;
    return imu_valid && abs(*omegaZ)>params.max_omegaZ;
}

int8_t nsteps=3;
enum AutoFireState updateAutoFire(const Track &tracked_object,
                           int16_t depth, int16_t hammer_intensity) {
    int32_t omegaZ=0;
    uint32_t now = micros();
    bool hit = false;
    bool lockout = omegaZLockout(&omegaZ);
    bool valid = tracked_object.valid(now);
    int32_t swing = 0;
    int32_t x=0, y=0;
    if(valid && !lockout) {
        swing=swingDuration(hammer_intensity)*1000;
        x=tracked_object.x;
        y=tracked_object.y;
        int32_t dt=swing/nsteps;
        for(int s=0;s<nsteps;s++) {
            tracked_object.project(dt, dt*omegaZ/1000000, &x, &y);
        }
        hit = (x>0) && (x/16<depth) && abs(y/16)<params.ytol;
    }
    enum AutoFireState st;
    if(lockout) st =     AF_OMEGAZ_LOCKOUT;
    else if(!valid) st = AF_NO_TARGET;
    else if(!hit) st =   AF_NO_HIT;
    else st =            AF_HIT;
    if(now - last_autofire_telem > params.autofire_telem_interval) {
        sendAutofireTelemetry(st, swing, x/16, y/16);
    }
    return st;
}

void setAutoFireParams(int16_t p_xtol,
                       int16_t p_ytol,
                       int16_t p_max_omegaz,
                       uint32_t telemetry_interval){
    params.xtol = p_xtol;
    params.ytol = p_ytol;
    params.max_omegaZ = p_max_omegaz;
    params.autofire_telem_interval = telemetry_interval;
    saveAutoFireParameters();
}

void saveAutoFireParameters(void) {
    eeprom_write_block(&params, &saved_params, sizeof(struct AutoFireParameters));
}

void restoreAutoFireParameters(void) {
    eeprom_read_block(&params, &saved_params, sizeof(struct AutoFireParameters));
}
