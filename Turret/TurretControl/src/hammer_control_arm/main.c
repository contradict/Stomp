#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sched.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <lcm/lcm.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

#include "toml.h"
#include "sclog4c/sclog4c.h"

#include "lcm_channels.h"
#include "lcm/stomp_control_radio.h"
#include "lcm/stomp_sensors_control.h"

#include "hammer_control_arm/toml_utils.h"
#include "hammer_control_arm/hammer_control_arm.h"

// -----------------------------------------------------------------------------
// globals
// -----------------------------------------------------------------------------

struct pru_config g_pru_config;

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

static const char* k_pru_channel_device = "/dev/rpmsg_pru31";

static const char* k_pru_rproc_device_root = "/sys/class/remoteproc/remoteproc5";
static const char* k_pru_firmware_root = "/lib/firmware";
static const char* k_pru_firmware = "am57xx-pru1_1-hammer_control";

static const uint32_t k_message_buff_len = 512;

static int k_message_type_strlen = 4;
static char* k_message_type_sync = "SYNC";
static char* k_message_type_exit = "EXIT";
static char* k_message_type_logm = "LOGM";

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static lcm_t *s_lcm;
static stomp_sensors_control_subscription_t* s_stomp_sensors_subscription;

static int s_rpmsg_fd;

static char s_message_buffer[512];

// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

static void init_sig_handlers();
static void init_lcm();
static void init_pru();
static void init_rpmsg();

static void rpmsg_handle(char *message);

static void stomp_sensors_control_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_sensors_control *lcm_msg, void *user);

static void shutdown();

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    char *config_filename = "../hammer_config.toml";

    // Parse command line
    
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

    // Read TOML config file
    
    FILE* fp;
    char errbuf[200];
    if (0 == (fp = fopen(config_filename, "r")))
    {
        snprintf(errbuf, sizeof(errbuf),"Unable to open config file %s:", config_filename);
        perror(errbuf);
        exit(1);
    }

    toml_table_t *toml_full_config = toml_parse_file(fp, errbuf, sizeof(errbuf));

    if (0 == toml_full_config)
    {
        logm(SL4C_FATAL, "Unable to parse %s: %s\n", config_filename, errbuf);
        exit(1);
    }

    toml_table_t *toml_hammer_config = toml_table_in(toml_full_config, "hammer_config");

    if (0 == toml_hammer_config)
    {
        logm(SL4C_FATAL, "No table 'hammer_config' in config.\n");
        exit(1);
    }

    toml_table_t *toml_pru_config = toml_table_in(toml_hammer_config, "pru_config");

    if (0 == toml_pru_config)
    {
        logm(SL4C_FATAL, "No table 'pru_config' in config.\n");
        exit(1);
    }

    if (parse_toml_pru_config(toml_pru_config) !=0)
    {
        logm(SL4C_FATAL, "could not parse pru_config message.\n");
        exit(1);
    }

    // Init all systems
    
    init_sig_handlers();
    init_lcm();

    init_pru();
    init_rpmsg();
    
    /*
    // Set our priority and scheduling algoritm.  Must be very aggressive to be able to
    // read analog inputs as quickly as possible

    struct sched_param sched = {
        .sched_priority = 10
    };

    if(sched_setscheduler(0, SCHED_FIFO, &sched) != 0)
    {
        perror("Unable to set scheduler");
        logm(SL4C_FATAL, "Try:\nsudo setcap \"cap_sys_nice=ep\" %s\n", argv[0]);
        return 0;
    }
    */

    //  Select needs highest fd + 1.  Calculate that here
    
    int max_fd;

    if (lcm_get_fileno(s_lcm) > s_rpmsg_fd)
    {
        max_fd = lcm_get_fileno(s_lcm) + 1;
    }
    else
    {
        max_fd = s_rpmsg_fd + 1;
    }

    // Read Sensor Inputs, process, and then publish to LCM

    int select_status;
    struct timeval timeout = { 0, 10000 };

    int lcm_fd = lcm_get_fileno(s_lcm);

    while(true)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(lcm_fd, &fds);
        FD_SET(s_rpmsg_fd, &fds);

        select_status = select(max_fd, &fds, NULL, NULL, &timeout);

        if (select_status >= 0)
        {
            // read and process lcm messages

            if (FD_ISSET(lcm_fd, &fds))
            {
                logm(SL4C_FINE, "lcm message received");
                lcm_handle(s_lcm);
            }

            // read and process RPMsg messages
            
            if (FD_ISSET(s_rpmsg_fd, &fds))
            {
                memset(&s_message_buffer, '\0', sizeof(k_message_buff_len));
                if (read(s_rpmsg_fd, s_message_buffer, k_message_buff_len) > 0)
                {
                    logm(SL4C_FINE, "rpmsg message received: %s", s_message_buffer);
                    rpmsg_handle(s_message_buffer);
                }
            }
        }
        else
        {
            logm(SL4C_FATAL, "Error %i from select(): %s", errno, strerror(errno));
        }
    }

    shutdown();
    return 0;
}

// -----------------------------------------------------------------------------
// SIGINT handler
// -----------------------------------------------------------------------------

void termination_handler (int signum)
{
    shutdown();

    logm(SL4C_DEBUG, "exit from signal handler");
    exit(2);
} 

void shutdown()
{
    // Shutdown LCM
    
    logm(SL4C_DEBUG, "Shutdown hammer_control_arm");
    stomp_sensors_control_unsubscribe(s_lcm, s_stomp_sensors_subscription);
    lcm_destroy(s_lcm);

    // Shutdown RPMSG
    
    logm(SL4C_DEBUG, "tell PRU to exit");
    write(s_rpmsg_fd, k_message_type_exit, k_message_type_strlen);

    close(s_rpmsg_fd);
}

// -----------------------------------------------------------------------------
// init methods
// -----------------------------------------------------------------------------

void init_sig_handlers()
{
    struct sigaction action;

    action.sa_handler = termination_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction (SIGINT, &action, NULL);
}

void init_lcm()
{
    // create lcm 

    s_lcm = lcm_create(NULL);

    if(!s_lcm)
    {
        logm(SL4C_FATAL, "Failed to initialize LCM.");
        exit(2);
    }

    // subscribe to the lcm messages we will bridge to an RPMsg

    s_stomp_sensors_subscription = stomp_sensors_control_subscribe(s_lcm, SENSORS_CONTROL, &stomp_sensors_control_handler, NULL);
}

void init_pru()
{
    char pru_rproc_device_state_filename[128];
    sprintf(pru_rproc_device_state_filename, "%s/state", k_pru_rproc_device_root);

    char pru_rproc_device_firmware_filename[128];
    sprintf(pru_rproc_device_firmware_filename, "%s/firmware", k_pru_rproc_device_root);

    char pru_firmware_filename[128];
    sprintf(pru_firmware_filename, "%s/%s", k_pru_firmware_root, k_pru_firmware);

    int pru_rproc_device_state_fd;
    int pru_rproc_device_firmware_fd;

    pru_rproc_device_state_fd = open(pru_rproc_device_state_filename, O_RDWR);

    if (pru_rproc_device_state_fd < 0)
    {
        logm(SL4C_FATAL, "Could not open %s", pru_rproc_device_state_filename);
        exit(1);
    }

    // Make sure firmware file exists

    struct stat st;

    if (stat(pru_firmware_filename, &st) < 0)
    {
        logm(SL4C_FATAL, "PRU Firmware does not exist: %s\n", pru_firmware_filename);
        exit(1);
    }
    else if (!S_ISREG(st.st_mode))
    {
        logm(SL4C_FATAL, "PRU Firmware exist but is not a normal file: %s\n", pru_firmware_filename);
        exit(1);
    }

    pru_rproc_device_firmware_fd = open(pru_rproc_device_firmware_filename, O_WRONLY);

    if (pru_rproc_device_firmware_fd < 0)
    {
        logm(SL4C_FATAL, "Could not open %s", pru_rproc_device_firmware_filename);
        exit(1);
    }

    // stop the PRU, first check status.  If running, then stop

    char buf[128];
    if (read(pru_rproc_device_state_fd, buf, 128) <= 0)
    {
        logm(SL4C_FATAL, "Could not read %s", pru_rproc_device_state_filename);
        exit(1);
    }

    if (strncmp(buf, "running", 7) == 0)
    {
        logm(SL4C_INFO, "Stopping %s", k_pru_rproc_device_root);
        write(pru_rproc_device_state_fd, "stop", 4);
    }

    // load the firmware to the PRU

    logm(SL4C_INFO, "Loading firmware %s", k_pru_firmware);

    write(pru_rproc_device_firmware_fd, k_pru_firmware, strlen(k_pru_firmware));
    close(pru_rproc_device_firmware_fd);

    // start the PRU

    logm(SL4C_INFO, "Starting %s", k_pru_rproc_device_root);

    write(pru_rproc_device_state_fd, "start", 5);
    close(pru_rproc_device_state_fd);

    // pause for 2 seconds before continuing
    usleep(2000000);
}

void init_rpmsg()
{
    memset(s_message_buffer, 0, sizeof(s_message_buffer));

    // open the character device channel to the pru

    s_rpmsg_fd = open(k_pru_channel_device, O_RDWR);

    if (s_rpmsg_fd < 0)
    {
        logm(SL4C_FATAL, "Could not open pru device file%s", k_pru_channel_device);
        exit(2);
    }

    // send the init message, this is how the PRU gets our  "address"

    logm(SL4C_FINE, "Send 'SYNC' message to PRU");

    if (write(s_rpmsg_fd, k_message_type_sync, k_message_type_strlen) < 0)
    {
        logm(SL4C_FATAL, "Could not send pru sync message. Error %i from write(): %s", errno, strerror(errno));
        exit(2);
    }
}

// -----------------------------------------------------------------------------
// lcm message handlers
// -----------------------------------------------------------------------------


void rpmsg_handle(char *message)
{
    char* current_char = message;

    // pull out the timestamp
    
    char* timestamp = current_char;

    while (*current_char != ':' && *current_char != 0) { current_char++; }
    *current_char = 0;
    current_char++;

    // pull out the message type
    
    char* message_type = current_char;

    while (*current_char != ':' && *current_char != 0) { current_char++; }
    *current_char = 0;
    current_char++;

    // pull out the message body
    
    char* body = current_char;

    if (strncmp(message_type, k_message_type_logm, k_message_type_strlen) == 0)
    {
        logm(SL4C_INFO, "%s - %s", timestamp, body);
    }
}

// -----------------------------------------------------------------------------
// lcm message handlers
// -----------------------------------------------------------------------------

static void stomp_sensors_control_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_sensors_control *lcm_msg, void *user)
{
    static struct timeval now;
    static struct timeval last_msg;
    gettimeofday(&now, 0);

    double microsec = ((now.tv_sec - last_msg.tv_sec) / 1000000.0f) + (now.tv_usec - last_msg.tv_usec);

    logm(SL4C_FINE, "%s msg received, dt = %f", channel, microsec);
    last_msg = now;

    // send the message down to the PRU

    char sensors_message_buff[k_message_buff_len];

    sprintf(sensors_message_buff, "SENS:HA:%d:%d:TA:%d:%d:TP:%d:RP:%d\n",
        (int32_t)lcm_msg->hammer_angle,
        (int32_t)lcm_msg->hammer_velocity,
        (int32_t)lcm_msg->turret_angle,
        (int32_t)lcm_msg->turret_velocity,
        (int32_t)lcm_msg->throw_pressure,
        (int32_t)lcm_msg->retract_pressure);

    logm(SL4C_DEBUG, "SEND RPMSG: %s", sensors_message_buff);

    if (strlen(sensors_message_buff) >= k_message_buff_len)
    {
        logm(SL4C_FATAL, "Message will not fit in maximum RPMsg message size");
        return;
    }

    if (write(s_rpmsg_fd, sensors_message_buff, strlen(sensors_message_buff)) < 0)
    {
        logm(SL4C_ERROR, "Could not send pru sens message. Error %i from write(): %s", errno, strerror(errno));
    }
}
