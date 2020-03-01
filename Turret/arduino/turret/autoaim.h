#pragma once
#include <stdint.h>
#include "track.h"

enum AutoAimState {
    AA_NO_TARGET = 0,
    AA_TRACKING_TARGET,
    AA_LOCKED_ON,
};

enum AutoAimState updateAutoAim(const Track &tracked_object);

int32_t desiredAutoAimTurretSpeed();

void setAutoAimParams(int16_t p_xtol,
                       int16_t p_ytol,
                       int16_t p_max_omegaz,
                       uint32_t telemetry_interval);

void restoreAutoAimParameters(void);
