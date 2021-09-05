#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <math.h>
#include <stdbool.h>

#include "sclog4c/sclog4c.h"

#include "modbus_device.h"
#include "leg_control/modbus_utils.h"
#include "leg_control/rate_timer.h"

const float minimum[3]={0.17, -0.070, -0.230};
const float maximum[3]={0.21,  0.070, -0.170};

const int nlegs=6;
const uint8_t leg_addresses[6] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60};

const float proportional_gain[3] = {12.0f, 12.0f, 12.0f};
const float force_damping[3] = {20.0f, 20.0f, 20.0f};

static bool zero_error(modbus_t* ctx, bool (*error_zero)[nlegs], const uint8_t *leg_addresses, const int nlegs)
{
    int err;
    float measured_toe_position[3];
    float ignored_pressures[6];
    bool done = true;
    for(int l=0; l<nlegs; l++)
    {
        if(!(*error_zero)[l])
        {
            err = get_toe_feedback(ctx, leg_addresses[l], &measured_toe_position, &ignored_pressures);
            if(err == -1)
            {
                done = false;
                continue;
            }
            err = set_toe_postion(ctx, leg_addresses[l], &measured_toe_position);
            if(err == -1)
            {
                done=false;
                continue;
            }
            (*error_zero)[l] = true;
        }
    }
    return done;
}

static bool set_gains(modbus_t* ctx, bool (*gain_set)[nlegs], const uint8_t *leg_addresses, const int nlegs)
{
    int err;
    bool done = true;
    for(int l=0; l<nlegs; l++)
    {
        if(!(*gain_set)[l])
        {
            err = set_servo_gains(ctx, leg_addresses[l], &proportional_gain, &force_damping);
            if(err == -1)
            {
                done = false;
                continue;
            }
            (*gain_set)[l] = true;
        }
    }
    return done;
}

static void compute_toe_position(float phase, float (*toe_position)[3])
{
    float alpha = (sinf(2.0f*M_PI*phase) + 1.0f) / 2.0f;
    float beta = (sinf(2.0f*M_PI*2.0f*phase) + 1.0f) / 2.0f;

    (*toe_position)[0] = minimum[0] + beta*(maximum[0] - minimum[0]);
    (*toe_position)[1] = minimum[1] + alpha*(maximum[1] - minimum[1]);
    (*toe_position)[2] = minimum[2] + beta*(maximum[2] - minimum[2]);
}

static int step(modbus_t *ctx, float phase, const uint8_t *leg_addresses, const int nlegs)
{
    int ret = 0, err;
    float toe_position[3];
    compute_toe_position(phase, &toe_position);
    for(int l=0;l<nlegs;l++)
    {
        err = set_toe_postion(ctx, leg_addresses[l], &toe_position);
        if(err == -1)
        {
            logm(SL4C_ERROR, "error moving leg 0x%02x: %s", leg_addresses[l],
                 modbus_strerror(errno));
            ret = -1;
        }
    }
    return ret;
}

static int count_false(bool *v, int n)
{
    int c=0;
    for(int i=0;i<n;i++)
        c += v[i] ? 0 : 1;
    return c;
}

int main(int argc, char **argv)
{
    char *devname="/dev/ttyS4";
    int baud = 1000000;
    float loop_frequency = 100.0f;
    float pattern_frequency = 0.1f;
    uint32_t byte_timeout = 20;
    uint32_t response_timeout = 10000;

    sclog4c_level = SL4C_INFO;

    int opt;
    while((opt = getopt(argc, argv, "p:f:w:r:t:d:")) != -1)
    {
        switch(opt)
        {
            case 'p':
                devname = strdup(optarg);
                break;
            case 'f':
                loop_frequency = atof(optarg);
                break;
            case 'w':
                pattern_frequency = atof(optarg);
                break;
            case 'r':
                response_timeout = 1000*atoi(optarg);
                break;
            case 't':
                byte_timeout = atoi(optarg);
                break;
            case 'd':
                sclog4c_level = atoi(optarg);
                logm(SL4C_FATAL, "Log level set to %d.", sclog4c_level);
                break;
        }
    }

    modbus_t* ctx;
    if(create_modbus_interface(devname, baud, byte_timeout, response_timeout, &ctx))
    {
        return 0;
    }

    struct sched_param sched = {
        .sched_priority = 10
    };
    if(sched_setscheduler(0, SCHED_FIFO, &sched) != 0)
    {
        perror("Unable to set scheduler");
        logm(SL4C_FATAL, "Try:\nsudo setcap \"cap_sys_nice=ep\" %s\n", argv[0]);
        //return 0;
    }

    bool error_zero[nlegs], gain_set[nlegs];

    bzero(error_zero, sizeof(error_zero));
    bzero(gain_set, sizeof(gain_set));

    struct rate_timer *timer = create_rate_timer(loop_frequency);
    logm(SL4C_DEBUG, "Create timer with period %f", loop_frequency);

    float elapsed = 0, dt=0, phase=0, discard;
    while(elapsed < 60)
    {
        sleep_rate(timer, &elapsed, &dt);
        phase = modff(phase + pattern_frequency * dt, &discard);
        logm(SL4C_FINE, "el: %6.3f dt: %5.3f", elapsed, dt);

        if(!zero_error(ctx, &error_zero, leg_addresses, nlegs))
        {
            logm(SL4C_INFO, "zero error %d", count_false(error_zero, nlegs));
            continue;
        }

        if(!set_gains(ctx, &gain_set, leg_addresses, nlegs))
        {
            logm(SL4C_INFO, "gain set %d", count_false(gain_set, nlegs));
            continue;
        }

        step(ctx, phase, leg_addresses, nlegs);
    }

    destroy_rate_timer(&timer);
 
    return 0;
}
