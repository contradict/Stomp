#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
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
    const unsigned int sbus_baud = 100000;
    const int sbus_pkt_length = 25; //complete sbus packet size
    const int pkt_timeout_usecs = 2000; //number of usecs
    struct timeval pkt_timeout;
    pkt_timeout.tv_sec = 0;
    pkt_timeout.tv_usec = pkt_timeout_usecs;

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

    tty.c_cc[VTIME] = 0;  //non-blocking reads
    tty.c_cc[VMIN] = 0;

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
    tty2.c_ispeed = sbus_baud;
    tty2.c_ospeed = sbus_baud;
    if (ioctl(serial_port, TCSETS2, &tty2) < 0)
    {
        printf("Error %i from ioctl: %s\n", errno, strerror(errno));
    }

    char read_buff [256];
    char sbus_pkt [256];
    fd_set rfds;  //file descriptor set you need to send to select
    int pkt_length;
    int num_bytes;
    int sret;
    while(true) //main loop, read sbus, then send data as lcm message
    {
        pkt_length = 0;
        memset(&read_buff, '\0', sizeof(read_buff));
        memset(&sbus_pkt, '\0', sizeof(sbus_pkt));
        while(true) //packet read loop
        { 
            FD_ZERO(&rfds);
            FD_SET(serial_port, &rfds);
            sret = select(serial_port + 1, &rfds, NULL, NULL, &pkt_timeout);
            if (sret  == 0) {
                break;   //timeout occurred, hopefully end of packet
            } else if (sret > 0)
            {
                num_bytes  = read(serial_port, &read_buff, sizeof(read_buff));
                if (num_bytes > 0)
                {
                    int i;
                    for (i = 0; i < num_bytes; i++)
                    {
                        sbus_pkt[pkt_length] = read_buff[i];
                        pkt_length++;
                    }
                } else {
                    printf("Error %i from read: %s\n", errno, strerror(errno));
                }
            } else {
                printf("Error %i from select %s\n", errno, strerror(errno));
            }

        }
        
        if (pkt_length == sbus_pkt_length) {
            printf("Complete SBUS packet received\n");
        } else if (pkt_length < sbus_pkt_length) {
            printf("Incomplete packet, %i bytes received\n", pkt_length);
            continue;
        } else {
            printf("Timeout missed?, %i bytes received\n", pkt_length);
            continue;
        }

        printf("First byte: %i, Last byte: %i, Second byte: %i\n", sbus_pkt[0], sbus_pkt[pkt_length - 1], sbus_pkt[1]);
                
        stomp_control_radio_publish(lcm, SBUS_RADIO_COMMAND, &radio_message);
    }

    lcm_destroy(lcm);
    return 0;
}
