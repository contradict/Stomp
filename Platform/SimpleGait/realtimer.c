#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>

#include "realtimer.h"

const long MinExtraSleep=1000000; // nanoseconds

void start_time(struct RealTimer *t)
{
    gettimeofday(&t->start, NULL);
    memcpy(&t->wakeup, &t->start, sizeof(struct timeval));
}

float elapsed_time(struct RealTimer *t)
{
    gettimeofday(&t->now, NULL);
    timersub(&t->now, &t->start, &t->elapsed);
    return t->elapsed.tv_sec + 1e-6f * t->elapsed.tv_usec;
}

void sleep_period(struct RealTimer *t, float period)
{
    float e = elapsed_time(t);
    if(e<period)
    {
        struct timespec req = {0, 0}, rem = {0, 0};
        float seconds;
        req.tv_nsec = modff(period - e, &seconds) * 1e9f; 
        req.tv_sec = seconds;
        while(1)
        {
            int err = nanosleep(&req, &rem);
            if(err == -1 && errno == EINTR && rem.tv_nsec > MinExtraSleep)
            {
                memcpy(&req, &rem, sizeof(struct timespec));
                continue;
            }
            break;
        }
    }
    gettimeofday(&t->wakeup, NULL);
}


