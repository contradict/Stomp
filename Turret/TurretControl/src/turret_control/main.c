#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/select.h>
#include <lcm/lcm.h>

#include "sclog4c/sclog4c.h"

#include "lcm_channels.h"
#include "lcm/stomp_control_radio.h"
#include "lcm/stomp_hammer_trigger.h"
#include "lcm/stomp_turret_telemetry.h"

#include "turret_control/turret_control.h"
#include "turret_control/lcm_handlers.h"

// -----------------------------------------------------------------------------
// global variables
// -----------------------------------------------------------------------------

lcm_t *g_lcm;
struct radio_control_parameters_t g_radio_control_parameters;

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

enum turret_state 
{
    TURRET_INVALID,
    TURRET_INIT,
    TURRET_SAFE,
    TURRET_ACTIVE
};

enum turret_rotation_state 
{
    ROTATION_INVALID,
    ROTATION_INIT,
    ROTATION_SAFE,
    ROTATION_MANUAL,
    ROTATION_AUTO,
    ROTATION_OVERRIDE
};

static enum turret_state s_turret_state = TURRET_INVALID;
static enum turret_rotation_state s_turret_rotation_state = ROTATION_INVALID;

static char *s_config_filename = "../turret_config.toml";

// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

void init();

void update();
void update_turret_state();
void update_turret_rotation_state();

void set_turret_state(enum turret_state state);
void set_turret_rotation_state(enum turret_rotation_state state);

void message_hammer_throw();
void message_hammer_retract();

void print_radio_control_parameters();

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    int opt;
    while((opt = getopt(argc, argv, "c:d:")) != -1)
    {
        switch(opt)
        {
            case 'c':
                s_config_filename = strdup(optarg);
                break;
            case 'd':
                sclog4c_level = atoi(optarg);
                logm(SL4C_FATAL, "Log level set to %d.", sclog4c_level);
                break;
        }
    }

    // Init state machines

    set_turret_state(TURRET_INIT);
    set_turret_rotation_state(ROTATION_INIT);

    //
    // Main Loop
    //

    while(true)
    {
        update();
    }

    // Shutdown

    control_radio_shutdown();
    lcm_destroy(g_lcm);

    return 0;
}

// -----------------------------------------------------------------------------
// private methods
// -----------------------------------------------------------------------------

void update()
{
    // Wait for a lcm_message to come in, and then handle it

    if (s_turret_state != TURRET_INIT && 
        s_turret_state != TURRET_INVALID)
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

    // Debug output the controller state, as we understand it.

    print_radio_control_parameters();

    // Update state machines
    
    update_turret_state();
    update_turret_rotation_state();

    // Apply Rotation
    
    if (s_turret_state == TURRET_ACTIVE)
    {
        switch (s_turret_rotation_state)
        {
            case ROTATION_MANUAL:
            case ROTATION_OVERRIDE:
                {
                }
                break;

            case ROTATION_AUTO:
                {
                }
                break;

            default:
                break;
        }
    }

    // Trigger hammer throw / retract ONLY in active (non-safe) state

    if (s_turret_state == TURRET_ACTIVE)
    {
        if (g_radio_control_parameters.hammer_trigger == HAMMER_TRIGGER_THROW)
        {
            message_hammer_throw();
        }
        else if (g_radio_control_parameters.hammer_trigger == HAMMER_TRIGGER_RETRACT)
        {
            message_hammer_retract();
        }
    }
}

void update_turret_state()
{
    while(true)
    {
        enum turret_state prev_turret_state = s_turret_state;

        switch (s_turret_state)
        {
            case TURRET_INIT:
                {
                    set_turret_state(TURRET_SAFE);
                }
                break;

            case TURRET_SAFE:
                {
                    if (g_radio_control_parameters.enable == TURRET_ENABLED)
                    {
                        set_turret_state(TURRET_ACTIVE);
                    }
                }
                break;
        
            case TURRET_ACTIVE:
                {
                    if (g_radio_control_parameters.enable != TURRET_ENABLED)
                    {
                        set_turret_state(TURRET_SAFE);
                    }
                }
                break;

            default:
                break;
        }

        if (s_turret_state == prev_turret_state)
        {
            break;
        }
    }
}


void update_turret_rotation_state()
{
    while(true)
    {
        enum turret_rotation_state prev_turret_rotation_state = s_turret_rotation_state;

        switch (s_turret_rotation_state)
        {
            case ROTATION_INIT:
                {
                    set_turret_rotation_state(ROTATION_SAFE);
                }
                break;

            case ROTATION_SAFE:
                {
                    if (s_turret_state == TURRET_ACTIVE)
                    {
                        if (g_radio_control_parameters.rotation_mode == ROTATION_MODE_MANUAL)
                        {
                            set_turret_rotation_state(ROTATION_MANUAL);
                        }
                        else if (g_radio_control_parameters.rotation_mode == ROTATION_MODE_AUTO)
                        {
                            set_turret_rotation_state(ROTATION_AUTO);
                        }
                    }
                }
                break;
        
            case ROTATION_MANUAL:
                {
                    if (s_turret_state != TURRET_ACTIVE)
                    {
                        set_turret_rotation_state(ROTATION_SAFE);
                    }
                    else if (g_radio_control_parameters.rotation_mode == ROTATION_MODE_AUTO)
                    {
                        set_turret_rotation_state(ROTATION_AUTO);
                    }
                }
                break;

            case ROTATION_AUTO:
                {
                    if (s_turret_state != TURRET_ACTIVE)
                    {
                        set_turret_rotation_state(ROTATION_SAFE);
                    }
                    else if (g_radio_control_parameters.rotation_mode == ROTATION_MODE_MANUAL)
                    {
                        set_turret_rotation_state(ROTATION_MANUAL);
                    }
                }
                break;

            case ROTATION_OVERRIDE:
                {
                }
                break;

            default:
                break;
        }

        if (s_turret_rotation_state == prev_turret_rotation_state)
        {
            break;
        }
    }
}

void set_turret_state(enum turret_state state)
{
    if (s_turret_state == state)
    {
        return;
    }

    logm(SL4C_DEBUG, "Set Turret State %d\n", state);

    // Handle exit state actions
    
    switch (s_turret_state)
    {
        default:
            break;
    }

    s_turret_state = state;

    // Handle enter state actions
    
    switch (s_turret_state)
    {
        case TURRET_INIT: 
            {
                init();
            }
            break;

        default:
            break;
    }
}

void set_turret_rotation_state(enum turret_rotation_state state)
{
    if (s_turret_rotation_state == state)
    {
        return;
    }

    logm(SL4C_DEBUG, "Set Turret Rotation State %d\n", state);

    // Handle exit state actions
    
    switch (s_turret_rotation_state)
    {
        default:
            break;
    }

    s_turret_rotation_state = state;

    // Handle enter state actions
    
    switch (s_turret_rotation_state)
    {
        default:
            break;
    }

}

void init()
{
    //
    // Read TOML config file
    //

    FILE* fp;
    char errbuf[200];
    if (0 == (fp = fopen(s_config_filename, "r")))
    {
        snprintf(errbuf, sizeof(errbuf),"Unable to open config file %s:", s_config_filename);
        perror(errbuf);
        exit(1);
    }

    toml_table_t *full_config = toml_parse_file(fp, errbuf, sizeof(errbuf));

    if (0 == full_config)
    {
        logm(SL4C_FATAL, "Unable to parse %s: %s\n", s_config_filename, errbuf);
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
}

void message_hammer_throw()
{
    stomp_hammer_trigger lcm_msg;

    lcm_msg.trigger_type = STOMP_HAMMER_TRIGGER_THROW_RETRACT;
    lcm_msg.throw_intensity = g_radio_control_parameters.throw_intensity;
    lcm_msg.retract_intensity = g_radio_control_parameters.retract_intensity;

    stomp_hammer_trigger_publish(g_lcm, HAMMER_TRIGGER, &lcm_msg);
}

void message_hammer_retract()
{
    stomp_hammer_trigger lcm_msg;

    lcm_msg.trigger_type = STOMP_HAMMER_TRIGGER_RETRACT_ONLY;
    lcm_msg.throw_intensity = g_radio_control_parameters.throw_intensity;
    lcm_msg.retract_intensity = g_radio_control_parameters.retract_intensity;

    stomp_hammer_trigger_publish(g_lcm, HAMMER_TRIGGER, &lcm_msg);
}

void print_radio_control_parameters()
{
    logm(SL4C_INFO, "Radio Control Parameters:\n\tEnabled: %s\n\tRot Mode: %s\n\tRot Intensity: %f\n\tHammer Trigger:%s\n\tThrow Intensity: %f\n\tRetract Intensity: %f\n",
            g_radio_control_parameters.enable == TURRET_ENABLED ? "TURRET_ENABLED" : "TURRET_DISABLED",
            g_radio_control_parameters.rotation_mode == ROTATION_MODE_DISABLED ? "ROTATION_MODE_DISABLED" : g_radio_control_parameters.rotation_mode == ROTATION_MODE_MANUAL ? "ROTATION_MODE_MANUAL" : "ROTATION_MODE_AUTO",
            g_radio_control_parameters.rotation_intensity,
            g_radio_control_parameters.hammer_trigger == HAMMER_SAFE ? "HAMMER_SAFE" : g_radio_control_parameters.hammer_trigger == HAMMER_TRIGGER_THROW ? "HAMMER_TRIGGER_THROW" : "HAMMER_TRIGGER_RETRACT",
            g_radio_control_parameters.throw_intensity,
            g_radio_control_parameters.retract_intensity);
}
