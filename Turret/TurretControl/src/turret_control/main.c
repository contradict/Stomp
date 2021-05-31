#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/select.h>
#include <lcm/lcm.h>

#include "sclog4c/sclog4c.h"

#include "lcm/stomp_control_radio.h"
#include "lcm/stomp_turret_telemetry.h"

#include "turret_control/turret_control.h"
#include "turret_control/lcm_handlers.h"

lcm_t *g_lcm;

int main(int argc, char **argv)
{
    char *config_filename = "../turret_config.toml";

    int opt;
    while((opt = getopt(argc, argv, "c:d:")) != -1)
    {
        switch(opt)
        {
            case 'c':
                config_filename = strdup(optarg);
                break;
            case 'd':
                sclog4c_level = atoi(optarg);
                logm(SL4C_FATAL, "Log level set to %d.", sclog4c_level);
                break;
        }
    }

    //
    // Read TOML config file
    //

    FILE* fp;
    char errbuf[200];
    if (0 == (fp = fopen(config_filename, "r")))
    {
        snprintf(errbuf, sizeof(errbuf),"Unable to open config file %s:", config_filename);
        perror(errbuf);
        exit(1);
    }

    toml_table_t *full_config = toml_parse_file(fp, errbuf, sizeof(errbuf));

    if (0 == full_config)
    {
        logm(SL4C_FATAL, "Unable to parse %s: %s\n", config_filename, errbuf);
        exit(1);
    }

    toml_table_t *robot_config = toml_table_in(full_config, "robot");

    if (0 == robot_config)
    {
        logm(SL4C_FATAL, "No table 'robot' in config.\n");
        exit(1);
    }

    char *robot_name;
    toml_raw_t tomlr = toml_raw_in(robot_config, "name");

    toml_rtos(tomlr, &robot_name);
    logm(SL4C_INFO, "Starting %s\n", robot_name);

    //
    // Setup LCM
    //
    
    g_lcm = lcm_create(NULL);
    if(!g_lcm)
    {
        logm(SL4C_FATAL, "Failed to initialize LCM.\n");
        exit(2);
    }

    //
    // Setup LCM Handlers
    //

    control_radio_init();

    //
    // Main Loop
    //

    while(true)
    {
        int lcm_fd = lcm_get_fileno(g_lcm);
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
            lcm_handle(g_lcm);
        }
    }

    control_radio_shutdown();
    lcm_destroy(g_lcm);
    return 0;
}
