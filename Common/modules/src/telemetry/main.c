#include <inttypes.h>
#include <unistd.h>
#include <lcm/lcm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>

#include "messages.h"
#include "telemetry.h"
#include "lcm_channels.h"
#include "sbus_channels.h"
#include "sclog4c/sclog4c.h"

#include "lcm_handlers.h"
#include "cosmos_handlers.h"
#include "rfd900x.h"

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static lcm_t *s_lcm;

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    sclog4c_level = SL4C_FATAL; //default logging, fatal errors only

    int opt; //get command line args
    while((opt = getopt(argc, argv, "vt:")) != -1)
    {
        switch(opt)
        {
            case 'v': //v for verbose, set log level to debug
                sclog4c_level = SL4C_DEBUG;
                break;
        }
    }

    telem_init();

    s_lcm = lcm_create(NULL);
    if (!s_lcm)
    {
        logm(SL4C_FATAL, "Failed to init LCM.");
        return 1;
    }

    stomp_control_radio_subscribe(s_lcm, SBUS_RADIO_COMMAND, &sbus_handler, NULL);

#ifdef HULL
    stomp_telemetry_leg_subscribe(s_lcm, LEG_TELEMETRY, &leg_handler, NULL);
#endif

#ifdef TURRET
    stomp_turret_telemetry_subscribe(s_lcm, TURRET_TELEMETRY, &turret_telemetry_handler, NULL);
    stomp_sensors_control_subscribe(s_lcm, SENSORS_CONTROL, &turret_sensors_control_handler, NULL);
#endif

    while (1)
    {
        int lcm_fd = lcm_get_fileno(s_lcm);
        int cosmons_fd = rfd900x_get_fileno();

        fd_set fds;
        FD_ZERO(&fds);
        if (lcm_fd >= 0) FD_SET(lcm_fd, &fds);
        if (cosmons_fd >= 0) FD_SET(cosmons_fd, &fds);

        int status = select(lcm_fd + 1, &fds, 0, 0, NULL);

        if (status > 0)
        {
            if (FD_ISSET(lcm_fd, &fds))
            {
                logm(SL4C_DEBUG, "Received LCM Message, forward to Cosmos");
                lcm_handle(s_lcm);
            }

            if (FD_ISSET(cosmons_fd, &fds))
            {
                cosmos_handle(s_lcm);
            }
        } 
        else 
        {
            logm(SL4C_FATAL, "Error %i from select(): %s", errno, strerror(errno));
        }
    }

    lcm_destroy(s_lcm);
    return 0;
}