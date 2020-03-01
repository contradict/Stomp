#pragma once
#include <stdint.h>
#include "track.h"

enum AutoFireState {
    AF_NO_TARGET = 0,
    AF_OMEGAZ_LOCKOUT,
    AF_NO_HIT,
    AF_HIT
};

enum AutoFireState updateAutoFire(const Track &tracked_object,
                           int16_t depth, int16_t hammer_intensity);

bool omegaZLockout(int16_t *omegaZ);

//  BB MJS: TMP
int32_t getTmpAdjustment();
//  BB MJS: END TMP

void setAutoFireParams(int16_t p_xtol,
                       int16_t p_ytol,
                       int16_t p_max_omegaz,
                       uint32_t telemetry_interval);

void restoreAutoFireParameters(void);
