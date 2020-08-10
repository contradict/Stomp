#include <inttypes.h>
#include <lcm/lcm.h>
#include <stdio.h>
#include "channel_names.h"
#include "lcm/stomp_control_radio.h"

static void my_handler(const lcm_recv_buf_t *rbuf, const char *channel,
                       const stomp_control_radio *msg, void *user)
{
    int i;
    int channels = sizeof msg->channel;
    printf("Received message on channel \"%s\":\n", channel);
    printf(" Chs: ");
    for (i = 0; i < channels; i++)
        printf(" %i:%.3f", i, msg->channel[i]);
    printf("\n");
    printf("  failsafe:%i'\n", msg->failsafe);
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
