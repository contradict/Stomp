#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "modbus_device.h"

#include "leg_control/modbus_utils.h"

#include "sclog4c/sclog4c.h"

int main(int argc, char** argv)
{
    char *devname = "/dev/ttyS4";
    uint32_t baud = 1000000;
    int32_t period = 100;
    uint32_t response_timeout = 2000;
    uint8_t address=0x55;

    int opt;
    while((opt = getopt(argc, argv, "a:d:b:p:r:v:")) != -1)
    {
        switch(opt)
        {
            case 'a':
                address = atol(optarg);
                break;
            case 'd':
                devname = strdup(optarg);
                break;
            case 'b':
                baud = atol(optarg);
                break;
            case 'p':
                period = atol(optarg);
                break;
            case 'r':
                response_timeout = 1000*atoi(optarg);
                break;
            case 'v':
                sclog4c_level = atoi(optarg);
                logm(SL4C_FATAL, "Log level set to %d.", sclog4c_level);
                break;
        }
    }

    modbus_t *ctx;
    if(create_modbus_interface(devname, baud, response_timeout, &ctx))
    {
        logm(SL4C_FATAL, "Unable to open modbus interface: %s",
                modbus_strerror(errno));
        return -1;
    }

    float zero_gain[3] = {0.0f, 0.0f, 0.0f};
    float zero_damping[3] = {0.0f, 0.0f, 0.0f};

    float real_gain[3] = {15.0f, 15.0f, 15.0f};
    float real_damping[3] = {2.0f, 2.0f, 2.0f};


    int loop_count=0;
    while(1)
    {
        int err = set_servo_gains(ctx, address, &zero_gain, &zero_damping);
        if(err == -1)
        {
            logm(SL4C_ERROR, "Unable to zero gains for leg 0x%02x.", address);
        }
        struct timespec req = {.tv_sec=0, .tv_nsec=period*1000000};
        nanosleep(&req, NULL);
        err = set_servo_gains(ctx, address, &real_gain, &real_damping);
        if(err == -1)
        {
            logm(SL4C_ERROR, "Unable to set gains for leg 0x%02x.", address);
        }
        nanosleep(&req, NULL);
        logm(SL4C_INFO, "Loop %d.", loop_count);
        loop_count++;
    }
 }
