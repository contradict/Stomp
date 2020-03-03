#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>

#include <linux/serial.h>

/* termios2 */
#include <asm/ioctls.h>
#include <asm/termbits.h>

/* Include definition for RS485 ioctls: TIOCGRS485 and TIOCSRS485 */
#include <sys/ioctl.h>

#include "modbus_crc.h"


int rs485_open(char *portname, uint32_t baudrate)
{
    /* Open your specific device (e.g., /dev/mydevice): */
    int fd = open (portname, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror("Open failed");
        return -1;
    }
    fcntl(fd, F_SETFL, 0);

    struct termios2 tio;
    // ioctl(fd, TCGETS2, &tio);
    bzero(&tio, sizeof(struct termios2));
    // tio.c_cflag &= ~CBAUD;
    // tio.c_cflag |= BOTHER;
    // tio.c_cflag &= ~PARENB | ~CSTOPB | ~CSIZE | ~HUPCL | ~CRTSCTS;
    // tio.c_cflag |= CS8 | CLOCAL | CREAD;
    tio.c_cflag = BOTHER | CS8 | CLOCAL | CREAD;
    tio.c_ospeed = baudrate;
    if(ioctl(fd, TCSETS2, &tio) < 0)
    {
        perror("Set speed failed");
        close(fd);
        return -1;
    }

    struct serial_rs485 rs485conf;

    /* Enable RS485 mode: */
    rs485conf.flags |= SER_RS485_ENABLED;

    /* Set logical level for RTS pin equal to 1 when sending: */
    // rs485conf.flags |= SER_RS485_RTS_ON_SEND;
    /* or, set logical level for RTS pin equal to 0 when sending: */
    rs485conf.flags &= ~(SER_RS485_RTS_ON_SEND);

    /* Set logical level for RTS pin equal to 1 after sending: */
    rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;
    /* or, set logical level for RTS pin equal to 0 after sending: */
    //rs485conf.flags &= ~(SER_RS485_RTS_AFTER_SEND);

    /* Set rts delay before send, if needed: */
    rs485conf.delay_rts_before_send = 0;

    /* Set rts delay after send, if needed: */
    rs485conf.delay_rts_after_send = 0;

    /* Set this flag if you want to receive data even while sending data */
    rs485conf.flags |= SER_RS485_RX_DURING_TX;

    if (ioctl (fd, TIOCSRS485, &rs485conf) < 0) {
        perror("Configure RS485 failed");
        close(fd);
        return -1;
    }
    return fd;
}

int main(int argc, char **argv)
{
    int fd = rs485_open("/dev/ttyS4", 1000000);
    if (fd < 0) {
        perror("Open failed");
        exit(1);
    }
    char data[6] = {0x55, 0x03, 0x55, 0x01, 0x00 , 0x00};
    char rdata[16];
    uint16_t cksum = modbus_crc(data, 4);
    printf("cksum: %04x\n", cksum);
    ((uint16_t *)data)[2] = cksum;
    for(int i=0;i<6;i++)
    {
        printf("%02x ", data[i]);
    }
    printf("\n");
    for(int i=0;i<50;i++)
    {
        printf("w=%d ", write(fd, data, 6));
        usleep(100);
        int32_t available;
        ioctl(fd, FIONREAD, &available);
        printf("a=%d ", available);
        if(available > 0)
        {
            read(fd, rdata, available);
            for(int j=0; j<available; j++)
            {
                printf("%02x ", rdata[j]);
            }
        }
        printf("\n");
        usleep(500000);
    }

    /* Close the device when finished: */
    if (close (fd) < 0) {
        perror("Close failed");
    }
}
