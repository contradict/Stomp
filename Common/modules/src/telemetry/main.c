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
#include "sclog4c/sclog4c.h"

#ifdef HULL
#include "lcm/stomp_telemetry_leg.h"
#endif

#ifdef TURRET
#include "lcm/stomp_turret_telemetry.h"
#include "lcm/stomp_sensors_control.h"
#endif

const int gnrl_period_msec = 200;  //general packet send period

#ifdef HULL
const int leg_period_msec = 200; //leg packet send period
#endif

#ifdef TURRET
const int turret_period_msec = 200; //turret packet send period
#endif

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

#ifdef HULL

int8_t m_to_int(float value){
    return (int8_t) value;
}

int8_t psi_to_int(float value){
    return (int8_t) value;
}

static void leg_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_telemetry_leg *msg, void *user)
{
    static struct timeval now;
    static struct timeval last_send_time;

    gettimeofday(&now, 0);
    int millis = time_diff_msec(last_send_time, now);
    logm(SL4C_DEBUG, "%d msec since last %s msg received", millis, channel);

    if (millis > leg_period_msec) //prepare and send COSMOS msg
    {
        gettimeofday(&last_send_time, 0);
        struct legs_cosmos cosmos_msg;
        logm(SL4C_DEBUG, "Sending COSMOS LEGS packet");
        int leg;
        for (leg = 0; leg < STOMP_TELEMETRY_LEG_LEGS; leg++)
        {
            int curl = leg + STOMP_TELEMETRY_LEG_CURL;
            int swing = leg + STOMP_TELEMETRY_LEG_SWING;
            int lift = leg + STOMP_TELEMETRY_LEG_LIFT;
            cosmos_msg.curl_base_psi[leg] = psi_to_int(msg->base_end_pressure[curl]);
            cosmos_msg.curl_rod_psi[leg] = psi_to_int(msg->rod_end_pressure[curl]);
            cosmos_msg.swing_base_psi[leg] = psi_to_int(msg->base_end_pressure[swing]);
            cosmos_msg.swing_rod_psi[leg] = psi_to_int(msg->rod_end_pressure[swing]);
            cosmos_msg.lift_base_psi[leg] = psi_to_int(msg->base_end_pressure[lift]);
            cosmos_msg.lift_rod_psi[leg] = psi_to_int(msg->rod_end_pressure[lift]);
            cosmos_msg.toe_pos_x[leg] = m_to_int(msg->toe_position_measured_X[leg]);
            logm(SL4C_DEBUG, "Leg: %d, toe_x_pos: %.4f, cosmos: %d", leg, msg->toe_position_measured_X[leg], cosmos_msg.toe_pos_x[leg]);
            cosmos_msg.toe_cmd_x[leg] = m_to_int(msg->toe_position_commanded_X[leg]);
            cosmos_msg.toe_pos_y[leg] = m_to_int(msg->toe_position_measured_Y[leg]);
            cosmos_msg.toe_cmd_y[leg] = m_to_int(msg->toe_position_commanded_Y[leg]);
            cosmos_msg.toe_pos_z[leg] = m_to_int(msg->toe_position_measured_Z[leg]);
            cosmos_msg.toe_cmd_z[leg] = m_to_int(msg->toe_position_commanded_Z[leg]);
        }
        cosmos_msg.observed_period = (uint8_t) (msg->observed_period*1000);
        telem_publish(LEGSTAT, (char *)&cosmos_msg, sizeof(cosmos_msg));
    }
}

#endif

#ifdef TURRET

static void turret_telemetry_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_turret_telemetry *msg, void *user)
{
    static struct timeval now;
    static struct timeval last_send_time;

    gettimeofday(&now, 0);
    int millis = time_diff_msec(last_send_time, now);
    logm(SL4C_DEBUG, "%d msec since last %s msg received", millis, channel);

    if (millis > turret_period_msec) //prepare and send COSMOS msg
    {
        gettimeofday(&last_send_time, 0);
        struct turret_gnrl_cosmos cosmos_msg;
        logm(SL4C_DEBUG, "Sending COSMOS Turret General packet");

        cosmos_msg.turret_state = msg->turret_state;

        telem_publish(TURRET_GNRL, (char *)&cosmos_msg, sizeof(cosmos_msg));
    }
}

static void turret_sensors_control_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_sensors_control *msg, void *user)
{
    static struct timeval now;
    static struct timeval last_send_time;

    gettimeofday(&now, 0);
    int millis = time_diff_msec(last_send_time, now);
    logm(SL4C_DEBUG, "%d msec since last %s msg received", millis, channel);

    if (millis > turret_period_msec) //prepare and send COSMOS msg
    {
        gettimeofday(&last_send_time, 0);
        struct turret_sensors_cosmos cosmos_msg;
        logm(SL4C_DEBUG, "Sending COSMOS Turret Sensors packet");

        cosmos_msg.hammer_angle = msg->hammer_angle;
        cosmos_msg.turret_angle = msg->turret_angle;
        cosmos_msg.throw_pressure = msg->throw_pressure;
        cosmos_msg.retract_pressure = msg->retract_pressure;

        telem_publish(TURRET_SNS, (char *)&cosmos_msg, sizeof(cosmos_msg));
    }
}

#endif

int main(int argc, char **argv)
{
    sclog4c_level = SL4C_FATAL; //default logging, fatal errors only
    telem_init();

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

#ifdef HULL
    stomp_telemetry_leg_subscribe(lcm, LEG_TELEMETRY, &leg_handler, NULL);
#endif

#ifdef TURRET
    stomp_turret_telemetry_subscribe(lcm, TURRET_TELEMETRY, &turret_telemetry_handler, NULL);
    stomp_sensors_control_subscribe(lcm, SENSORS_CONTROL, &turret_sensors_control_handler, NULL);
#endif

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
