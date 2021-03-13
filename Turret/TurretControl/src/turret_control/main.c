#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/select.h>

#include "turret_control.h"

#include <lcm/lcm.h>

#include "sclog4c/sclog4c.h"

#include "lcm/stomp_control_radio.h"
#include "lcm/stomp_turret_control.h"

int main(int argc, char **argv)
{
    int opt;
    while((opt = getopt(argc, argv, "d:")) != -1)
    {
        switch(opt)
        {
            case 'd':
                sclog4c_level = atoi(optarg);
                logm(SL4C_FATAL, "Log level set to %d.", sclog4c_level);
                break;
        }
    }

    lcm_t *lcm = lcm_create(NULL);
    if(!lcm)
    {
        logm(SL4C_FATAL, "Failed to initialize LCM.\n");
        exit(2);
    }

    while(true)
    {
        int lcm_fd = lcm_get_fileno(lcm);
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(lcm_fd, &fds);

        struct timeval timeout = {
            0,
            10000,
        };

        int status = select(lcm_fd + 1, &fds, 0, 0, &timeout);
        if(status == 1 && FD_ISSET(lcm_fd, &fds))
        {
            lcm_handle(lcm);
        }
    }

    lcm_destroy(lcm);
    return 0;
}
