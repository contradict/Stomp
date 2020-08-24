#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "messages.h"
#include "telemetry.h"
#include "sclog4c/sclog4c.h"

int main(int argc, char **argv)
{
    sclog4c_level = SL4C_FATAL; //default logging, fatal errors only
    int sleep_time = 1;

    int opt; //get command line args
    while((opt = getopt(argc, argv, "vt:")) != -1)
    {
        switch(opt)
        {
            case 'v': //v for verbose, set log level to debug
                sclog4c_level = SL4C_DEBUG;
                break;
            case 't':
                sleep_time = atoi(optarg);
        }
    }
    telem_init();
    struct tank_psi msg;
    uint32_t psi = 0;
    while (1) {
        logm(SL4C_DEBUG, "Sending COSMOS test packet");
        msg.psi = psi++;
        telem_publish(TANK_PSI, (char *)&msg, sizeof(msg));
        sleep(sleep_time);
    }

    return 0;
}
