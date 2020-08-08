#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/select.h>

#include <lcm/lcm.h>

#include "lcm/stomp_control_radio.h"
#include "lcm/stomp_modbus.h"
#include "lcm/stomp_telemetry_leg.h"
#include "leg_control/queue.h"
#include "leg_control/leg_thread.h"
#include "leg_control/control_radio.h"
#include "leg_control/toml_utils.h"

int main(int argc, char **argv)
{
    struct leg_thread_definition leg_thread;
    leg_thread.devname = "/dev/ttyS4";
    leg_thread.baud = 1000000;
    leg_thread.period = 10;
    leg_thread.response_timeout = 2000;
    char *config_filename = "hull_config.toml";

    int opt;
    while((opt = getopt(argc, argv, "p:b:t:r:c:")) != -1)
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
            case 'c':
                config_filename = strdup(optarg);
                break;
        }
    }

    FILE* fp;
    char errbuf[200];
    if(0 == (fp = fopen(config_filename, "r")))
    {
        snprintf(errbuf, sizeof(errbuf),"Unable to open config file %s:", config_filename);
        perror(errbuf);
        exit(1);
    }

    toml_table_t *full_config = toml_parse_file(fp, errbuf, sizeof(errbuf));
    if(0 == full_config)
    {
        printf("Unable to parse %s: %s\n", config_filename, errbuf);
        exit(1);
    }
    leg_thread.config = toml_table_in(full_config, "robot");
    if(0 == leg_thread.config)
    {
        printf("No table 'robot' in config.\n");
        exit(1);
    }
    toml_raw_t tomlr = toml_raw_in(leg_thread.config, "name");
    char *robot_name;
    toml_rtos(tomlr, &robot_name);
    printf("Starting %s\n", robot_name);
    
    lcm_t *lcm = lcm_create(NULL);
    if(!lcm)
    {
        printf("Failed to initialize LCM.\n");
        exit(2);
    }

    leg_thread.lcm = lcm;

    create_queue(1,  3*sizeof(stomp_control_radio), &leg_thread.parameter_queue);
    create_queue(1,  10*sizeof(stomp_modbus), &leg_thread.command_queue);
    create_queue(1,  10*sizeof(stomp_modbus), &leg_thread.response_queue);
    create_queue(1,  10*sizeof(stomp_telemetry_leg), &leg_thread.telemetry_queue);
    struct leg_thread_state *state = create_leg_thread(&leg_thread, argv[0]);
    if(state==0)
        exit(3);

    struct control_radio_state radio_state;
    radio_state.lcm = lcm;
    radio_state.parameter_queue.buffer = leg_thread.parameter_queue.buffer;
    radio_state.parameter_queue.ringbuf = leg_thread.parameter_queue.ringbuf;
    int err= control_radio_init(&radio_state);
    if(err)
    {
        terminate_leg_thread(&state);
        exit(err);
    }

    while(true)
    {
        int lcm_fd = lcm_get_fileno(lcm);
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(lcm_fd, &fds);

        struct timeval timeout = {
            0,
            100000,
        };
        int status = select(lcm_fd + 1, &fds, 0, 0, &timeout);
        if(0 == status)
        {
            // check telemetry and response queues
        }
        else if(FD_ISSET(lcm_fd, &fds))
        {
            lcm_handle(lcm);
        }
    }

    control_radio_shutdown(&radio_state);
    terminate_leg_thread(&state);

    lcm_destroy(lcm);
    return 0;
}
