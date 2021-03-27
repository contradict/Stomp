#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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

#include "sclog4c/sclog4c.h"

#include "lcm_channels.h"
#include "lcm/stomp_control_radio.h"
#include "lcm/stomp_sensors_control.h"

#include "lcm_rpmsg_bridge/lcm_rpmsg_bridge.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

static const char* k_pru_channel_device = "/dev/rpmsg_pru31";

static const char* k_pru_rproc_device_root = "/sys/class/remoteproc/remoteproc5";
static const char* k_pru_firmware_root = "/lib/firmware";
static const char* k_pru_firmware = "am57xx-pru1_1-hammer_control";

static const uint32_t k_message_buff_len = 512;

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static lcm_t *s_lcm;
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

static void turret_sensors_control_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_sensors_control *lcm_msg, void *user);

static void shutdown();

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    // parse command line
    
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

    init_sig_handlers();
    init_lcm();

    init_pru();
    init_rpmsg();
    
    //  select needs highest fd + 1.  Calculate that here
    
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
    logm(SL4C_DEBUG, "Shutdown lcm_rpmsg_bridge");
    lcm_destroy(s_lcm);

    logm(SL4C_DEBUG, "tell PRU to exit");
    write(s_rpmsg_fd, "EXIT", 4);
    
    logm(SL4C_DEBUG, "Read all rpmsg");

    int32_t bytes_read = 0;
    do
    {
        bytes_read = read(s_rpmsg_fd, s_message_buffer, k_message_buff_len);
        logm(SL4C_DEBUG, "shutdown: drop rpmsg - %s", s_message_buffer);
    } while (bytes_read <= 0);

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

    stomp_sensors_control_subscribe(s_lcm, SENSORS_CONTROL, &turret_sensors_control_handler, NULL);
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

    if (write(s_rpmsg_fd, "SYNC", 9) < 0)
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
}

// -----------------------------------------------------------------------------
// lcm message handlers
// -----------------------------------------------------------------------------

static void turret_sensors_control_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_sensors_control *lcm_msg, void *user)
{
    static struct timeval now;
    static struct timeval last_msg;
    gettimeofday(&now, 0);

    double microsec = ((now.tv_sec - last_msg.tv_sec) / 1000000.0f) + (now.tv_usec - last_msg.tv_usec);

    logm(SL4C_FINE, "%s msg received, dt = %f", channel, microsec);

    // send the message down to the PRU

    last_msg = now;

    if (write(s_rpmsg_fd, "SENS", 4) < 0)
    {
        logm(SL4C_ERROR, "Could not send pru sens message. Error %i from write(): %s", errno, strerror(errno));
    }

    //  debug output
    
    logm(SL4C_DEBUG, "\nSENSOR_CONTROL packet:\n\thammer_angle_valid: %d\n\thammer_angle = %f\n\tturret_angle_valid: %d\n\tturret_angle = %f\n\tthrow_pressure_valid: %d\n\tthrow_pressure = %f\n\tretract_pressure_valid: %d\n\tretract_pressure = %f",
        lcm_msg->hammer_angle_valid,
        lcm_msg->hammer_angle,
        lcm_msg->turret_angle_valid,
        lcm_msg->turret_angle,
        lcm_msg->throw_pressure_valid,
        lcm_msg->throw_pressure,
        lcm_msg->retract_pressure_valid,
        lcm_msg->retract_pressure);
}


