#include <time.h>
#include <bsd/sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "leg_control/rate_timer.h"

struct rate_timer {
    float rate;
    struct timespec increment, start_time, request, wake_time, elapsed;
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

float sleep_rate(struct rate_timer* timer)
{
    timespecadd(&timer->wake_time, &timer->increment, &timer->request);
    while(EINTR==clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timer->request, NULL));
    clock_gettime(CLOCK_MONOTONIC, &timer->wake_time);
    timespecsub(&timer->wake_time, &timer->start_time, &timer->elapsed);
    return timer->elapsed.tv_sec + timer->elapsed.tv_nsec;
}

void destroy_rate_timer(struct rate_timer** timer)
{
    free(*timer);
    *timer = NULL;
}
