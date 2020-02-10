#pragma once
#include <stdint.h>
#include "object.h"

struct Track
{
    int32_t x, vx, y, vy;
    int32_t rx, ry;
    uint32_t num_updates;
    uint32_t last_update, last_predict;
    int32_t last_omgaz;

    int16_t alpha, beta;  // position, velocity filter
    uint32_t track_lost_dt; // timeout for no observations
    uint32_t min_num_updates; // minimum number before trusted
    int32_t max_off_track; // squared distance in mm
    int32_t max_start_distance; // squared distance in mm


    // Default constructor
    Track();
    void project(int32_t dt, int32_t dtheta, int32_t *px, int32_t *py) const;
    int32_t predict(uint32_t now, int16_t omegaZ);
    void update(const Object& best_match);
    int32_t distanceSq(const Object& obj) const;
    bool recent_update(uint32_t now) const;
    bool valid(uint32_t now) const;
    int16_t updateOmegaZ(int32_t dt, int16_t omegaZ);
    void updateNoObs(uint32_t inter_leddar_time, int16_t omegaZ);
    bool wants_update(uint32_t now, int32_t best_distance);
    int32_t angle(void) const;
    int32_t vtheta(void) const;
    void setTrackingFilterParams(int16_t alpha, int16_t beta,
                             int8_t p_min_num_updates,
                             uint32_t p_track_lost_dt,
                             int16_t p_max_off_track,
                             int16_t p_max_start_distance);
    void saveTrackingFilterParams(void);
    void restoreTrackingFilterParams(void);
};


