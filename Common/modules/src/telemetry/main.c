#include <inttypes.h>
#include <lcm/lcm.h>
#include <stdio.h>
#include <sys/time.h>
#include "lcm_channels.h"
#include "sbus_channels.h"
#include "lcm/stomp_control_radio.h"

static void my_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_control_radio *msg, void *user)
{
    int i;
    int axes = STOMP_CONTROL_RADIO_AXES;
    int toggles = STOMP_CONTROL_RADIO_TOGGLES;
    struct timeval now;
    gettimeofday(&now, 0);
    printf("Received message on channel %s, at %ld.%ld\n", channel, now.tv_sec, now.tv_usec);
    printf(" Axes: ");
    for (i = 0; i < axes; i++)
        printf(" %i:%.3f", i, msg->axis[i]);
    printf("\n");
    for (i = 0; i < toggles; i++)
    {
        switch(msg->toggle[i])
        {
            case STOMP_CONTROL_RADIO_ON:
                printf(" %i:%s", i, "ON");
                break;
            case STOMP_CONTROL_RADIO_CENTER:
                printf(" %i:%s", i, "CENTER");
                break;
            case STOMP_CONTROL_RADIO_OFF:
                printf(" %i:%s", i, "OFF");
                break;

        }
    }
    printf("\n");
    printf("  failsafe:%i\n", msg->failsafe);
    printf("  no_data:%i\n", msg->no_data);
}

int main(int argc, char **argv)
{
    lcm_t *lcm = lcm_create(NULL);
    if (!lcm)
        return 1;

    stomp_control_radio_subscribe(lcm, SBUS_RADIO_COMMAND, &my_handler, NULL);

    while (1)
        lcm_handle(lcm);

    lcm_destroy(lcm);
    return 0;
}
