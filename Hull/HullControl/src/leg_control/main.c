#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/wait.h>

#include <lcm/lcm.h>

#include "lcm/stomp_control_radio.h"
#include "lcm/stomp_modbus.h"
#include "lcm/stomp_telemetry_leg.h"
#include "leg_control/queue.h"
#include "leg_control/leg_thread.h"
#include "leg_control/control_radio.h"

int main(int argc, char **argv)
{
    struct leg_thread_state leg_thread;
    leg_thread.shouldrun = true;
    leg_thread.devname = "/dev/ttyS4";
    leg_thread.baud = 1000000;
    leg_thread.period = 10;
    leg_thread.response_timeout = 2000;

    int opt;
    while((opt = getopt(argc, argv, "p:b:t:r:")) != -1)
    {
        switch(opt)
        {
            case 'p':
                leg_thread.devname = strdup(optarg);
                break;
            case 'b':
                leg_thread.baud = atol(optarg);
                break;
            case 't':
                leg_thread.period = atoi(optarg);
                break;
            case 'r':
                leg_thread.response_timeout = 1000*atoi(optarg);
                break;
        }
    }

    lcm_t *lcm = lcm_create(NULL);
    if(!lcm)
    {
        printf("Failed to initialize LCM.\n");
        return 1;
    }

    leg_thread.lcm = lcm;

    create_queue(1,  3*sizeof(stomp_control_radio), &leg_thread.parameter_queue);
    create_queue(1,  10*sizeof(stomp_modbus), &leg_thread.command_queue);
    create_queue(1,  10*sizeof(stomp_modbus), &leg_thread.response_queue);
    create_queue(1,  10*sizeof(stomp_telemetry_leg), &leg_thread.telemetry_queue);
    pid_t leg_thread_pid = create_leg_thread(&leg_thread, argv[0]);
    if(leg_thread_pid<0)
        exit(leg_thread_pid);

    struct control_radio_state radio_state;
    radio_state.lcm = lcm;
    radio_state.parameter_queue.buffer = leg_thread.parameter_queue.buffer;
    radio_state.parameter_queue.ringbuf = leg_thread.parameter_queue.ringbuf;
    int err= control_radio_init(&radio_state);
    if(err)
    {
        int lt_status;
        terminate_leg_thread(&leg_thread);
        waitpid(leg_thread_pid, &lt_status, 0);
        exit(err);
    }

    while(true)
        lcm_handle(lcm);

    control_radio_shutdown(&radio_state);
    int status;
    terminate_leg_thread(&leg_thread);
    waitpid(leg_thread_pid, &status, 0);

    lcm_destroy(lcm);
    return 0;
}
