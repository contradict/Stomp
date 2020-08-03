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

    //open and configure the serial port
    printf("Trying to open UART10\n");

    int serial_port;
    serial_port = open("/dev/ttyS1", O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_port < 0)
    {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    } else {
        printf("No error while opening port\n");
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty); //create termios struct and set to zero
    if (tcgetattr(serial_port, &tty) != 0) //read current port config
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }

    tty.c_cflag &= ~PARENB; //N
    tty.c_cflag |= CS8;     //8
    tty.c_cflag &= ~CSTOPB; //1
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

    tty.c_cc[VTIME] = 20;
    tty.c_cc[VMIN] = 0;

    cfsetispeed(&tty, 100000);
    cfsetospeed(&tty, 100000);

    //try to set the configuration of the serial port
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }

    //read loop
    char read_buffer [256];
    memset(&read_buffer, '\0', sizeof(read_buffer));
    while(true)
    {
        //Read from serial port, put data in radio_message
        int num_bytes  = read(serial_port, &read_buffer, sizeof(read_buffer));
        if (num_bytes == 0) {
            printf("No bytes in 2 seconds\n");
        } else if (num_bytes > 0) {
            printf("Read %i bytes\n", num_bytes);
        } else {
            printf("Error %i from read: %s\n", errno, strerror(errno));
        }

         stomp_control_radio_publish(lcm, SBUS_RADIO_COMMAND, &radio_message);
    }

    lcm_destroy(lcm);
    return 0;
}
