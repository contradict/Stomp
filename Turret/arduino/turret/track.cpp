#include <Arduino.h>
#include "track.h"
#include "utils.h"

struct TrackingFilterParameters {
    int16_t alpha, beta;  // position, velocity filter
    uint32_t track_lost_dt; // timeout for no observations
    uint32_t min_num_updates; // minimum number before trusted
    int32_t max_off_track; // squared distance in mm
    int32_t max_start_distance; // squared distance in mm
} __attribute__((packed));


struct TrackingFilterParameters EEMEM saved_tracking_params = {
    .alpha = 9000,
    .beta = 8192,
    .track_lost_dt = 250000,
    .min_num_updates = 3,
    .max_off_track = 1000L*1000L,
    .max_start_distance = 6000L*6000L
};


Track::Track() :
        x(0), vx(0),
        y(0), vy(0),
        num_updates(0),
        last_update(micros()),
        last_predict(micros()),
        alpha(10000), beta(10000),
        track_lost_dt(250000),
        min_num_updates(3),
        max_off_track(1000L*1000L),
        max_start_distance(6000L*6000L)
        { }

bool Track::recent_update(uint32_t now) const {
    uint32_t dt = (now - last_update);
    return dt<track_lost_dt;
}


// distance squared in mm
int32_t Track::distanceSq(const Object &detection) const {
    int32_t dx = (x/16-detection.xcoord());
    int32_t dy = (y/16-detection.ycoord());
    return dx*dx + dy*dy;
}

// return change in body angle as measured by gyro
// radians scaled by 2048
int16_t Track::updateOmegaZ(int32_t dt, int16_t omegaZ) {
    // work with radians/sec scaled by 32768
    // (2000deg/sec)/(32768 full scale)*pi/180*32768 = 34.9
    int32_t converted = omegaZ*35;
    int32_t average_omegaZ = (last_omgaz + converted)/2;
    last_omgaz = converted;
    return ((average_omegaZ/16)*(dt/1000))/1000;
}

void Track::project(int32_t dt, int32_t dtheta, int32_t *px, int32_t *py) const {
    // predict:
    // r = sqrt(x**2+y**2)
    // theta = atan2(y, x)
    // x = x + dt*vx/1e6 + r*(cos(theta-dtheta) - cos(theta))
    // x = x + dt*vx/1e6 + r*(cos(theta)*cos(dtheta) + sin(theta)*sin(dtheta) - x/r)
    // x = x + dt*vx/1e6 + r*((x/r)*cos(dtheta) + (y/r)*sin(dtheta) - x/r)
    // x = x + dt*vx/1e6 + (x*cos(dtheta) + y*sin(dtheta) - x)
    // x = x + dt*vx/1e6 + (x*(cos(dtheta)-1) + y*sin(dtheta))
    // x = x + dt*vx/1e6 + (x*(-dtheta*dtheta/2048/2/2048) + y*dtheta/2048)
    // x = x + dt*vx/1e6 - (x*dtheta*dtheta/2048/2 - y*dtheta)/2048
    int32_t lx=*px;
    int32_t ly=*py;
    *px = lx + ((dt/1000)*vx)/1000 - ((lx*dtheta/1024L)*dtheta/4L - ly*dtheta)/2048L;
    // y = y + dt*vy/1e6 + r*(sin(theta-dtheta) - sin(theta));
    // y = y + dt*vy/1e6 + r*(sin(theta)*cos(dtheta) - cos(theta)*sin(dtheta) - sin(theta));
    // y = y + dt*vy/1e6 + r*((y/r)*cos(dtheta) - (x/r)*sin(dtheta) - (y/r));
    // y = y + dt*vy/1e6 + (y*cos(dtheta) - x*sin(dtheta) - y);
    // y = y + dt*vy/1e6 + (y*(cos(dtheta)-1) - x*sin(dtheta));
    // y = y + dt*vy/1e6 - (y*(dtheta*dtheta/2048/2) + x*dtheta)/2048;
    *py = ly + ((dt/1000)*vy)/1000 - ((ly*dtheta/1024L)*dtheta/4L + lx*dtheta)/2048L;
}

int32_t Track::predict(uint32_t now, int16_t omegaZ) {
    int32_t dt = (now - last_predict);
    last_predict = now;
    int32_t dtheta = updateOmegaZ(dt, omegaZ);
    project(dt, dtheta, &x, &y);
    return dt;
}

void Track::update(const Object& best_match) {
    int32_t mx = best_match.xcoord();
    int32_t my = best_match.ycoord();
    if(!recent_update(best_match.Time)) {
        x = mx*16;
        y = my*16;
        vx = 0;
        vy = 0;
        num_updates = 0;
    } else {
        //
        // residual:
        // rx = mr*cos(ma) - x
        // rx = mr*(2048 - ma*ma/2048/2)/2048 - x
        rx = mx*16 - x;
        if(rx>65535L) rx=65535L;
        if(rx<-65535L) rx=-65535L;
        // ry = mr*sin(ma) - y
        ry = my*16 - y;
        if(ry>65535L) ry=65535L;
        if(ry<-65535L) ry=-65535L;
        //
        // correct:
        x += alpha*rx/32767;
        y += alpha*ry/32767;
        vx += beta*rx/4096;
        vx = clip(vx, -10000L*16, 10000L*16);
        vy += beta*ry/4096;
        vy = clip(vy, -10000L*16, 10000L*16);
    }
    num_updates++;
    last_update = best_match.Time;
}

void Track::updateNoObs(uint32_t time, int16_t omegaZ) {
    if(recent_update(time)) {
        predict(time, omegaZ);
    }
}

bool Track::wants_update(uint32_t now, int32_t best_distance) {
    return (( recent_update(now) && best_distance < max_off_track) ||
            (!recent_update(now) && best_distance < max_start_distance));
}

bool Track::valid(uint32_t now) const {
    return recent_update(now) && (num_updates>min_num_updates);
}

int32_t Track::angle(void) const {
    if(valid(micros())) {
        // arctan(y/x)
        return ((y*2048L)/x - ((((((y*(y/16L))/x)*y)/x)*2048L)/x)*16L/3L);
    } else {
        return 0;
    }
}

int32_t Track::vtheta(void) const {
    if(valid(micros())) {
        // d/dt arctan(y/x) = (1/(1+(y/x)**2))*(vy/x - y*vx/x**2)
        // (vy*x-vx*y)/(x**2+y**2)
        return vy*2048/x; // (vy*x - vx*y)*128L/(x*x/16L + y*y/16L);
    } else {
        return 0;
    }
}

void Track::setTrackingFilterParams(int16_t p_alpha, int16_t p_beta,
                             int8_t p_min_num_updates,
                             uint32_t p_track_lost_dt,
                             int16_t p_max_off_track,
                             int16_t p_max_start_distance
        ) {
    min_num_updates    = p_min_num_updates;
    track_lost_dt      = p_track_lost_dt;
    max_off_track      = (int32_t)p_max_off_track*p_max_off_track;
    max_start_distance = (int32_t)p_max_start_distance*p_max_start_distance;
    alpha = p_alpha;
    beta = p_beta;
    saveTrackingFilterParams();
}

void Track::saveTrackingFilterParams(void) {
    struct TrackingFilterParameters p;
    p.alpha = alpha;
    p.beta = beta;
    p.min_num_updates = min_num_updates;
    p.track_lost_dt = track_lost_dt;
    p.max_off_track = max_off_track;
    p.max_start_distance = max_start_distance;
    eeprom_write_block(&p, &saved_tracking_params, sizeof(struct TrackingFilterParameters));
}

void Track::restoreTrackingFilterParams(void) {
    struct TrackingFilterParameters p;
    eeprom_read_block(&p, &saved_tracking_params, sizeof(struct TrackingFilterParameters));
    alpha = p.alpha;
    beta = p.beta;
    min_num_updates = p.min_num_updates;
    track_lost_dt = p.track_lost_dt;
    max_off_track = p.max_off_track;
    max_start_distance = p.max_start_distance;
 }
