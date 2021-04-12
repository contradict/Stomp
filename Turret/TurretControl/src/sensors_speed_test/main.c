#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static int s_analog_01_in_fd = -1;
static int s_analog_02_in_fd = -1;
static int s_analog_03_in_fd = -1;
static int s_max_fd = -1;

static int s_num_devices = 1;

static float s_ms_total = 0.0f;
static int s_reads_total = 0;

static int s_analog_01_bytes_read = 0;
static int s_analog_02_bytes_read = 0;
static int s_analog_03_bytes_read = 0;
static int s_analog_01_selects = 0;
static int s_analog_02_selects = 0;
static int s_analog_03_selects = 0;

// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

void init_sig_handlers();
static void init_sensors();

float time_diff_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec)*1000 + ((float)(t1.tv_usec - t0.tv_usec))/1000.0f;
}

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    int opt;

    while((opt = getopt(argc, argv, "n:")) != -1)
    {
        switch(opt)
        {
            case 'n':
                s_num_devices = atoi(optarg);

                if (s_num_devices <= 0 || s_num_devices >= 4)
                {
                    printf("Invalid number of devices %d.  Must be between 1 and 3\n", s_num_devices);
                    return 0;
                }
                break;
        }
    }

    init_sig_handlers();
    init_sensors();

    // Read Sensor Inputs, process, and then publish to LCM

    int select_status;
    struct timeval timeout = { 0, 10000 };
    char read_buff[256];
    int num_bytes;

    /*
    struct sched_param sched = {
        .sched_priority = 10
    };

    if(sched_setscheduler(0, SCHED_FIFO, &sched) != 0)
    {
        perror("Unable to set scheduler");
        return 0;
    }
    */

    printf("Ctrl-C to exit and get timing info\n");

    static struct timeval now;
    static struct timeval last_send_time;

    gettimeofday(&now, 0);
    last_send_time = now;

    while(1)
    {
        gettimeofday(&now, 0);
        float ms = ((now.tv_sec - last_send_time.tv_sec) * 1000) + ((float)(now.tv_usec - last_send_time.tv_usec))/1000.0f;
        //printf("%f msec since last analog read\n", ms);
        last_send_time = now;

        s_ms_total += ms;
        s_reads_total++;

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(s_analog_01_in_fd, &fds);

        if (s_num_devices >= 2)
        {
            FD_SET(s_analog_02_in_fd, &fds);
        }

        if (s_num_devices >= 2)
        {
            FD_SET(s_analog_03_in_fd, &fds);
        }

        select_status = select(s_max_fd, &fds, NULL, NULL, &timeout);

        if (select_status > 0)
        {
            if (FD_ISSET(s_analog_01_in_fd, &fds))
            {
                memset(&read_buff, '\0', sizeof(read_buff));
                num_bytes = read(s_analog_01_in_fd, &read_buff, sizeof(read_buff));
                s_analog_01_bytes_read += num_bytes;
                s_analog_01_selects++;
                lseek(s_analog_01_in_fd, 0, SEEK_SET);
            }
            if (s_num_devices >= 2 && FD_ISSET(s_analog_02_in_fd, &fds))
            {
                memset(&read_buff, '\0', sizeof(read_buff));
                num_bytes = read(s_analog_02_in_fd, &read_buff, sizeof(read_buff));
                s_analog_02_bytes_read += num_bytes;
                s_analog_02_selects++;
                lseek(s_analog_02_in_fd, 0, SEEK_SET);
            }
            if (s_num_devices >= 3 && FD_ISSET(s_analog_03_in_fd, &fds))
            {
                memset(&read_buff, '\0', sizeof(read_buff));
                num_bytes = read(s_analog_03_in_fd, &read_buff, sizeof(read_buff));
                s_analog_03_bytes_read += num_bytes;
                s_analog_03_selects++;
                lseek(s_analog_03_in_fd, 0, SEEK_SET);
            }
        }
    }

    return 0;
}

void termination_handler (int signum)
{
    float ave_ms = s_ms_total / s_reads_total;

    printf("\nReading %d analog devices\n\n", s_num_devices);
    printf("Alalog device 1 bytes read = %d\n", s_analog_01_bytes_read);
    printf("Alalog device 1 selects = %d\n", s_analog_01_selects);
    printf("Alalog device 2 bytes read = %d\n", s_analog_02_bytes_read);
    printf("Alalog device 2 selects = %d\n", s_analog_02_selects);
    printf("Alalog device 3 bytes read = %d\n", s_analog_03_bytes_read);
    printf("Alalog device 3 selects = %d\n\n", s_analog_03_selects);
    printf("Average MS per analog read: %f\n", ave_ms);
    exit(2);
} 


void init_sig_handlers()
{
    struct sigaction action;

    action.sa_handler = termination_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction (SIGINT, &action, NULL);
}

void init_sensors()
{
    s_analog_01_in_fd = open("/sys/bus/iio/devices/iio:device0/in_voltage7_raw", O_RDONLY);

    if (s_analog_01_in_fd < 0)
    {
        exit(2);
    } 
    else if (s_analog_01_in_fd > s_max_fd)
    {
        s_max_fd = s_analog_01_in_fd;
    }

    if (s_num_devices >= 2)
    {
        s_analog_02_in_fd = open("/sys/bus/iio/devices/iio:device0/in_voltage4_raw", O_RDONLY);

        if (s_analog_02_in_fd < 0)
        {
            exit(2);
        } 
        else if (s_analog_02_in_fd > s_max_fd)
        {
            s_max_fd = s_analog_02_in_fd;
        }
    }

    if (s_num_devices >= 3)
    {
        s_analog_03_in_fd = open("/sys/bus/iio/devices/iio:device0/in_voltage6_raw", O_RDONLY);

        if (s_analog_03_in_fd < 0)
        {
            exit(2);
        } 
        else if (s_analog_03_in_fd > s_max_fd)
        {
            s_max_fd = s_analog_03_in_fd;
        }
    }
    
    //  select takes highest fd + 1 so increment s_max_fd

    if (s_max_fd < 0)
    {
        exit(2);
    } 

    s_max_fd++;

    printf("max_fd = %d\n", s_max_fd);
}

