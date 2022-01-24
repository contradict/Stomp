#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
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

static int32_t k_turret_rotation_scale = 1000;
static int32_t k_turret_rotation_min_threshold = 32;

static int32_t k_roboteq_baud_rate = 115200;

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

enum turret_state 
{
    TURRET_INVALID = -1,
    TURRET_INIT = 0,
    TURRET_SAFE = 1,
    TURRET_ACTIVE = 2
};

enum turret_rotation_state 
{
    ROTATION_INVALID = -1,
    ROTATION_INIT = 0,
    ROTATION_SAFE = 1,
    ROTATION_MANUAL = 2,
    ROTATION_AUTO = 3,
    ROTATION_OVERRIDE = 4
};

static enum turret_state s_turret_state = TURRET_INVALID;
static enum turret_rotation_state s_turret_rotation_state = ROTATION_INVALID;

static char *s_config_filename = "../turret_config.toml";

static int s_roboteq_fd;

// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

void init();
void init_roboteq();

void update();
void update_turret_state();
void update_turret_rotation_state();

void set_turret_state(enum turret_state state);
void set_turret_rotation_state(enum turret_rotation_state state);

void apply_turret_rotation(int32_t rotation);

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

    control_radio_handler_shutdown();
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
                    apply_turret_rotation(g_radio_control_parameters.rotation_intensity * k_turret_rotation_scale);
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
    
    //
    // Send out Turret Telemetry
    //

    stomp_turret_telemetry lcm_msg;
    lcm_msg.turret_state = s_turret_state;
    lcm_msg.rotation_state = s_turret_rotation_state;

    turret_telemetry_send(&lcm_msg);
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
                    if (s_turret_state != TURRET_ACTIVE || g_radio_control_parameters.rotation_mode == ROTATION_MODE_DISABLED)
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
                    if (s_turret_state != TURRET_ACTIVE || g_radio_control_parameters.rotation_mode == ROTATION_MODE_DISABLED)
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
  
    control_radio_handler_init();

    //
    // Setup roboteq controller
    // 

    init_roboteq();
}

void init_roboteq()
{
    //open UART10

    s_roboteq_fd = open("/dev/ttyS1", O_RDWR | O_NONBLOCK | O_NOCTTY);

    if (s_roboteq_fd < 0)
    {
        logm(SL4C_FATAL, "Error %i from open(): %s", errno, strerror(errno));
        return;
    } 

    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(s_roboteq_fd, &tty) != 0)
    {
        logm(SL4C_WARNING, "Error %i from tcgetattr: %s", errno, strerror(errno));
    }

    cfsetospeed (&tty, k_roboteq_baud_rate);
    cfsetispeed (&tty, k_roboteq_baud_rate);

    tty.c_cflag |= PARENB;  //E
    tty.c_cflag |= CS8;     //8
    tty.c_cflag |= CSTOPB;  //2
    tty.c_cflag &= ~CRTSCTS; //disable hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; //Turn on READ $ ignore ctrl lines

    tty.c_lflag &= ~ICANON; //non canonical
    tty.c_lflag &= ~ECHO; //disable echo
    tty.c_lflag &= ~ECHOE; //disable erasure
    tty.c_lflag &= ~ECHONL; //disable new-line echo
    tty.c_lflag &= ~ISIG; //disable interpretation of INTR, QUIT and SUSP

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); //disable software flow control
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|ICRNL); //disable special chars

    tty.c_oflag &= ~OPOST; //prevent special interpretation of output bytes
    tty.c_oflag &= ~ONLCR; //prevent nl converstion to cr

    tty.c_cc[VTIME] = 0; 
    tty.c_cc[VMIN] = 0;

    //try to set the configuration of the serial port

    if (tcsetattr(s_roboteq_fd, TCSANOW, &tty) != 0)
    {
        logm(SL4C_WARNING, "Error %i from tcsetattr: %s", errno, strerror(errno));
    }

    char roboteq_msg[64];

    // set serial priority first
    sprintf(roboteq_msg, "@00^CPRI 1 0");
    write(s_roboteq_fd, roboteq_msg, strlen(roboteq_msg));

    // set RC priority second
    sprintf(roboteq_msg, "@00^CPRI 2 1");
    write(s_roboteq_fd, roboteq_msg, strlen(roboteq_msg));

    // turn off command echo
    sprintf(roboteq_msg, "@00^ECHOF 1");
    write(s_roboteq_fd, roboteq_msg, strlen(roboteq_msg));

    // set RS232 watchdog to 100 ms
    sprintf(roboteq_msg, "@00^RWD 100");
    write(s_roboteq_fd, roboteq_msg, strlen(roboteq_msg));
}


void apply_turret_rotation(int32_t rotation)
{
    char roboteq_msg[64];

    if (s_roboteq_fd < 0)
    {
        return;
    }

    // Enforce a dead zone where for small values we just pass 0 rotation
    
    if (rotation >= -k_turret_rotation_min_threshold && 
        rotation <= k_turret_rotation_min_threshold)
    {
        rotation = 0;
    }

    //  send "@nn!G mm" over software serial. mm is a command 
    //  value, -1000 to 1000. nn is node number in RoboCAN network.

    sprintf(roboteq_msg, "@01!G %d", rotation);
    write(s_roboteq_fd, roboteq_msg, strlen(roboteq_msg));
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
