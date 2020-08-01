#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include <modbus.h>

#include "modbus_device.h"

int main(int argc, char **argv)
{
    char *devname = "/dev/ttyS4";
    uint32_t baud = 1000000;
    int period = 100;
    uint32_t response_timeout = 2000;

    int opt;
    while((opt = getopt(argc, argv, "p:b:a:t:r:")) != -1)
    {
        switch(opt)
        {
            case 'p':
                devname = strdup(optarg);
                break;
            case 'b':
                baud = atol(optarg);
                break;
            case 't':
                period = atoi(optarg);
                break;
            case 'r':
                response_timeout = 1000*atoi(optarg);
                break;
        }
    }

    modbus_t *ctx;

    struct sched_param sched = {
        .sched_priority = 10
    };
    if(sched_setscheduler(0, SCHED_FIFO, &sched) != 0)
    {
        perror("Unable to set scheduler");
        printf("Try:\nsudo setcap \"cap_sys_nice=ep\" %s\n", argv[0]);
        exit(1);
    }

    if(create_modbus_interface(devname, baud, response_timeout, &ctx))
    {
        exit(1);
    }

    return 0;
}
