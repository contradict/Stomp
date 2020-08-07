#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stropts.h>

#define termios asmtermios
#include <asm/termios.h>
#undef termios
#include <termios.h>

#include <lcm/lcm.h>

#include "channel_names.h"
#include "lcm/stomp_control_radio.h"

int main(int argc, char **argv)
{
    uint32_t custom_baud = 100000;

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
    serial_port = open("/dev/ttyS1", O_RDWR | O_NOCTTY);
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

    tty.c_cflag |= PARENB;  //E
    tty.c_cflag |= CS8;     //8
    tty.c_cflag |= CSTOPB;  //2
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

    //fix all these magic numbers soon
    tty.c_cc[VTIME] = 20;  //wait up to 2 seconds
    tty.c_cc[VMIN] = 25;  //return if 25 bytes arrive

    //try to set the configuration of the serial port
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }

    //set custom baud ratei
    struct termios2 tty2;
    ioctl(serial_port, TCGETS2, &tty2);
    tty2.c_cflag &= ~CBAUD;
    tty2.c_cflag |= BOTHER;
    tty2.c_ispeed = custom_baud;
    tty2.c_ospeed = custom_baud;
    if (ioctl(serial_port, TCSETS2, &tty2) < 0)
    {
        printf("Error %i from ioctl: %s\n", errno, strerror(errno));
    }

    //read loop
    char read_buffer [256];
    memset(&read_buffer, '\0', sizeof(read_buffer));
    while(true)
    {
        //Read from serial port, put data in radio_message
        int num_bytes  = read(serial_port, &read_buffer, sizeof(read_buffer));
        if (num_bytes == 0) {
            printf("No packet in 2 seconds\n");
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
