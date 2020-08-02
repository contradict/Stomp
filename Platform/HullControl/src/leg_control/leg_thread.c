#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "modbus_device.h"
#include "leg_control/leg_thread.h"

int run_leg_thread(struct leg_thread_state *leg_thread)
{
    while(leg_thread->shouldrun)
    {
    }
    return 0;
}

pid_t create_leg_thread(struct leg_thread_state *leg_thread, const char * progname)
{
    if(create_modbus_interface(leg_thread->devname, leg_thread->baud, leg_thread->response_timeout, &leg_thread->ctx))
    {
        return -1;
    }

    struct sched_param sched = {
        .sched_priority = 10
    };
    if(sched_setscheduler(0, SCHED_FIFO, &sched) != 0)
    {
        perror("Unable to set scheduler");
        printf("Try:\nsudo setcap \"cap_sys_nice=ep\" %s\n", progname);
        exit(1);
    }

    pid_t pid = fork();
    if(pid == 0)
        run_leg_thread(leg_thread);
    return pid;
}
