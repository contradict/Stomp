#include <stdbool.h>

#include <lcm/lcm.h>

#include "channel_names.h"
#include "lcm/stomp_control_radio.h"

int main(int argc, char **argv)
{
    lcm_t *lcm = lcm_create(NULL);
    if(!lcm)
    {
        printf("Failed to initialize LCM.\n");
        return 1;
    }

    stomp_control_radio radio_message;

    while(true)
    {
        //Read from serial port, put data in radio_message

        stomp_control_radio_publish(lcm, SBUS_RADIO_COMMAND, &radio_message);
    }

    lcm_destroy(lcm);
    return 0;
}
