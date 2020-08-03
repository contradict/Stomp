#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

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

    printf("Trying to open UART10\n");

    int serial_port;
    serial_port = open("/dev/ttyS1", O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_port < 0)
    {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    } else {
        printf("No error while opening port\n");
    }

    while(true)
    {
        //Read from serial port, put data in radio_message

        stomp_control_radio_publish(lcm, SBUS_RADIO_COMMAND, &radio_message);
    }

    lcm_destroy(lcm);
    return 0;
}
