#pragma once

#include <sys/time.h>

struct RealTimer
{
    struct timeval start;
    struct timeval now;
    struct timeval elapsed;
    struct timeval wakeup;
};


void start_time(struct RealTimer *t);
float elapsed_time(struct RealTimer *t);
void sleep_period(struct RealTimer *t, float period);
