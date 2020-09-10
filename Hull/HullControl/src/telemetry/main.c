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
#include "lcm/stomp_control_radio.h"
#include "lcm/stomp_telemetry_leg.h"
#include "sclog4c/sclog4c.h"

const int gnrl_period_msec = 200;  //general packet send period
const int leg_period_msec = 200; //leg packet send period

int time_diff_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec)*1000 + (t1.tv_usec - t0.tv_usec)/1000;
}

static void sbus_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_control_radio *msg, void *user)
{
    static struct timeval now;
    static struct timeval last_send_time;

    gettimeofday(&now, 0);
    logm(SL4C_DEBUG, "Received message on channel %s, at %ld.%ld\n", channel, now.tv_sec, now.tv_usec);
    logm(SL4C_DEBUG, "Data size: %d\n", rbuf->data_size);
 
    logm(SL4C_DEBUG, "  failsafe:%i\n", msg->failsafe);
    logm(SL4C_DEBUG, "  no_data:%i\n", msg->no_data);
    gettimeofday(&now, 0);
    int millis = time_diff_msec(last_send_time, now);
    if (millis > 1000)
    {
    }
}

static void leg_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_telemetry_leg *msg, void *user)
{
    static struct timeval now;
    static struct timeval last_send_time;

    gettimeofday(&now, 0);
    logm(SL4C_DEBUG, "Received message on channel %s, at %ld.%ld\n", channel, now.tv_sec, now.tv_usec);
    logm(SL4C_DEBUG, "Data size: %d\n", rbuf->data_size);

    logm(SL4C_DEBUG, "Observed period: %.4f", msg->observed_period);

    int millis = time_diff_msec(last_send_time, now);
    if (millis > leg_period_msec)
    {
        logm(SL4C_DEBUG, "New leg LCM msg received. %d msec since last COSMOS msg sent", millis);
        gettimeofday(&last_send_time, 0);
    }

}

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

    lcm_t *lcm = lcm_create(NULL);
    if (!lcm)
    {
        logm(SL4C_FATAL, "Failed to init LCM.");
        return 1;
    }

    stomp_control_radio_subscribe(lcm, SBUS_RADIO_COMMAND, &sbus_handler, NULL);
    stomp_telemetry_leg_subscribe(lcm, LEG_TELEMETRY, &leg_handler, NULL);

    while (1)
    {
        int lcm_fd = lcm_get_fileno(lcm);
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(lcm_fd, &fds);
        logm(SL4C_DEBUG, "Waiting forever for LCM message");
        int status = select(lcm_fd + 1, &fds, 0, 0, NULL);
        if (status <= 0 ){
            logm(SL4C_FATAL, "Error %i from select(): %s", errno, strerror(errno));
        } else {
            lcm_handle(lcm);
        }
    }

    lcm_destroy(lcm);
    return 0;
}
