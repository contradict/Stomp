#include <time.h>
#include <bsd/sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "leg_control/rate_timer.h"

struct rate_timer {
    float rate;
    struct timespec increment, start_time, wake_time;
};

struct rate_timer* create_rate_timer(float rate)
{
    struct rate_timer *timer = malloc(sizeof(struct rate_timer));
    timer->rate = rate;
    float fpart, ipart;
    fpart = modff(1.0f/rate, &ipart);
    timer->increment.tv_sec = truncf(ipart);
    timer->increment.tv_nsec = truncf(1e9f * fpart);
    restart_rate_timer(timer);
    return timer;
}

void restart_rate_timer(struct rate_timer* timer)
{
    clock_gettime(CLOCK_MONOTONIC, &timer->start_time);
    memcpy(&timer->wake_time, &timer->start_time, sizeof(struct timespec));
}

void sleep_rate(struct rate_timer* timer, float *elapsed, float *dt)
{
    struct timespec request;
    timespecadd(&timer->wake_time, &timer->increment, &request);
    while(EINTR==clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &request, NULL));
    struct timespec wake;
    clock_gettime(CLOCK_MONOTONIC, &wake);
    if(dt)
    {
        struct timespec delta;
        timespecsub(&wake, &timer->wake_time, &delta);
        *dt = delta.tv_sec + delta.tv_nsec * 1e-9f;
    }
    memcpy(&timer->wake_time, &wake, sizeof(struct timespec));
    if(elapsed)
    {
        struct timespec el;
        timespecsub(&timer->wake_time, &timer->start_time, &el);
        *elapsed = el.tv_sec + el.tv_nsec * 1e-9f;
    }
}

void destroy_rate_timer(struct rate_timer** timer)
{
    free(*timer);
    *timer = NULL;
}
