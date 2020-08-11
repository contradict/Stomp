#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <stropts.h>

#define termios asmtermios
#include <asm/termios.h>
#undef termios
#include <termios.h>

#include <lcm/lcm.h>

#include "channel_names.h"
#include "lcm/stomp_control_radio.h"

int time_diff_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec)*1000 + (t1.tv_usec - t0.tv_usec)/1000;
}


int main(int argc, char **argv)
{
    const unsigned int sbus_baud = 100000;
    const int sbus_pkt_length = 25; //complete sbus packet size
    const int sbus_ch_cnt = 16;
    const int sbus_max = 1811;
    const int sbus_min = 172;
    bool debug_out = false;
    const int pkt_timeout_usec = 2000; //number of usecs to wait for serial data
    int sbus_timeout_msec = 500; //number of millis before calling SBUS dead

    int opt; //get command line args
    while((opt = getopt(argc, argv, "v:t")) != -1)
    {
        switch(opt)
        {
            case 'v':
                debug_out = true;
                break;
            case 't':
                sbus_timeout_msec = atoi(optarg);
        }
    }

    struct timeval pkt_timeout;
    pkt_timeout.tv_sec = 0;
    pkt_timeout.tv_usec = pkt_timeout_usec;
    struct timeval last_packet_time;
    struct timeval read_time;

    lcm_t *lcm = lcm_create(NULL);
    if(!lcm)
    {
        printf("Failed to initialize LCM.\n");
        return 1;
    }
    stomp_control_radio lcm_msg;

    //open and configure the serial port
    printf("Trying to open UART10\n");

    int serial_port;
    serial_port = open("/dev/ttyS1", O_RDWR | O_NONBLOCK | O_NOCTTY);
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

    tty.c_cc[VTIME] = 0; 
    tty.c_cc[VMIN] = 0;

    //try to set the configuration of the serial port
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }

    //set custom baud rate
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

    uint8_t read_buff [256];
    uint8_t sbus_pkt [256];
    fd_set rfds;  //file descriptor set you need to send to select
    int pkt_length;
    int num_bytes;
    int sret;
    uint16_t sbus_raw[sbus_ch_cnt];
    float sbus_ch[sbus_ch_cnt];
    float sbus_span = (sbus_max - sbus_min)/2.0f;
    float sbus_center = (sbus_max + sbus_min)/2.0f;
    bool good_packet = false;
    bool failsafe = true;
    gettimeofday(&last_packet_time, 0);
    while(true) //main loop, read sbus, then send data as lcm message
    {
        pkt_length = 0;
        memset(&sbus_pkt, '\0', sizeof(sbus_pkt));
        failsafe = true;
        good_packet = false;
        while(true) //packet read loop
        {
            FD_ZERO(&rfds);
            FD_SET(serial_port, &rfds);
            pkt_timeout.tv_usec = pkt_timeout_usec;
            sret = select(serial_port + 1, &rfds, NULL, NULL, &pkt_timeout);
            if (sret > 0) //data available
            {
                memset(&read_buff, '\0', sizeof(read_buff));
                num_bytes  = read(serial_port, &read_buff, sizeof(read_buff));
                printf("Read returned with %i bytes\n", num_bytes);
                if (num_bytes > 0)
                {
                    int i;
                    for (i = 0; i < num_bytes; i++)
                    {
                        sbus_pkt[pkt_length] = read_buff[i];
                        pkt_length++;
                        if (pkt_length == sizeof(sbus_pkt)) break; 
                        //don't run off the end of the array
                    }

                } else {
                    printf("Error %i from read: %s\n", errno, strerror(errno));
                }
            } else if (sret  == 0) {
                printf("Select() timeout occured\n");
            } else {
                printf("Error %i from select %s\n", errno, strerror(errno));
            }

            if (pkt_length == sbus_pkt_length)
            {
                if (sbus_pkt[0] == 0x0F && sbus_pkt[24] == 0x00)
                {
                    gettimeofday(&last_packet_time, 0);
                    good_packet = true;
                    printf("Complete SBUS packet received\n");
                    break; //leave read loop and send
                } else {
                    printf("Malformed SBUS packet received\n");
                }
            } else if (pkt_length < sbus_pkt_length) {
                printf("Incomplete packet, %i bytes received\n", pkt_length);
                continue; //wait for more bytes 
            } else {
                printf("Oversize packet, %i bytes received\n", pkt_length);
            }

            gettimeofday(&read_time, 0);
            if (time_diff_msec(last_packet_time, read_time) > sbus_timeout_msec)
            {
                failsafe = true;
                printf("Timeout waiting for SBUS");
                break; //break and send zero packet
            }

            // get here if too much data or bad packet
            pkt_length = 0;
            memset(&sbus_pkt, '\0', sizeof(sbus_pkt));

        } //end packet read loop;

        if (good_packet) //if pkt is good, process, otherwise set failsafe
        {
            //convert SBUS format into channel values as ints
            //low bits come in first byte, high bits in next byte, litte endian
            sbus_raw[0]  = (sbus_pkt[2]  << 8  | sbus_pkt[1])                           & 0x07FF; // 8, 3
            sbus_raw[1]  = (sbus_pkt[3]  << 5  | sbus_pkt[2] >> 3)                      & 0x07FF; // 6, 5
            sbus_raw[2]  = (sbus_pkt[5]  << 10 | sbus_pkt[4] << 2 | sbus_pkt[3] >> 6)   & 0x07FF; // 1, 8, 2
            sbus_raw[3]  = (sbus_pkt[6]  << 7  | sbus_pkt[5] >> 1)                      & 0x07FF; // 4, 7
            sbus_raw[4]  = (sbus_pkt[7]  << 4  | sbus_pkt[6] >> 4)                      & 0x07FF; // 7, 4
            sbus_raw[5]  = (sbus_pkt[9]  << 9  | sbus_pkt[8] << 1 | sbus_pkt[7] >> 7)   & 0x07FF; // 2, 8, 1
            sbus_raw[6]  = (sbus_pkt[10] << 6  | sbus_pkt[9] >> 2)                      & 0x07FF; // 5, 6
            sbus_raw[7]  = (sbus_pkt[11] << 3  | sbus_pkt[10] >> 5)                     & 0x07FF; // 8, 3
            sbus_raw[8]  = (sbus_pkt[13] << 8  | sbus_pkt[12])                          & 0x07FF; // 3, 8
            sbus_raw[9]  = (sbus_pkt[14] << 5  | sbus_pkt[13] >> 3)                     & 0x07FF; // 6, 5
            sbus_raw[10] = (sbus_pkt[16] << 10 | sbus_pkt[15] << 2 | sbus_pkt[14] >> 6) & 0x07FF; // 1, 8, 2
            sbus_raw[11] = (sbus_pkt[17] << 7  | sbus_pkt[16] >> 1)                     & 0x07FF; // 4, 7
            sbus_raw[12] = (sbus_pkt[18] << 4  | sbus_pkt[17] >> 4)                     & 0x07FF; // 7, 4
            sbus_raw[13] = (sbus_pkt[20] << 9  | sbus_pkt[19] << 1 | sbus_pkt[18] >> 7) & 0x07FF; // 2, 8, 1
            sbus_raw[14] = (sbus_pkt[21] << 6  | sbus_pkt[20] >> 2)                     & 0x07FF; // 5, 6
            sbus_raw[15] = (sbus_pkt[22] << 3  | sbus_pkt[21] >> 5)                     & 0x07FF; // 8, 3
            failsafe = sbus_pkt[23] & 0x08;
        } else {
            memset(&sbus_pkt, '\0', sizeof(sbus_pkt));
            failsafe = true;
        }

        int i;
        for (i = 0; i < sbus_ch_cnt; i++)
        {
            sbus_ch[i] = (sbus_raw[i] - sbus_center)/sbus_span;
            lcm_msg.channels[i] = sbus_ch[i];
        }
        lcm_msg.failsafe = failsafe;

        printf("Channel 1: %i, Channel 2: %i, Failsafe: %i\n", sbus_raw[0], sbus_raw[1], failsafe);
        printf("Channel 1: %.4f, Channel 2: %.4f\n", sbus_ch[0], sbus_ch[1]);

        stomp_control_radio_publish(lcm, SBUS_RADIO_COMMAND, &lcm_msg);
    }

    lcm_destroy(lcm);
    return 0;
}
