#pragma once
#include <stdint.h>
#include "track.h"

enum AutofireState {
    AF_NO_TARGET = 0,
    AF_OMEGAZ_LOCKOUT,
    AF_NO_HIT,
    AF_HIT
};

enum AutofireState willHit(const Track &tracked_object,
                           int16_t depth, int16_t hammer_intensity);

bool omegaZLockout(int16_t *omegaZ);

void setAutoFireParams(int16_t p_xtol,
                       int16_t p_ytol,
                       int16_t p_max_omegaz,
                       uint32_t telemetry_interval);

void restoreAutofireParameters(void);
