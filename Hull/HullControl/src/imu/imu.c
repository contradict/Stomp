#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include "spidevlib.h"


int yost_read(int fd, uint8_t command, int length, uint8_t* data)
{
    char txbuf[length];
    memset(txbuf, 0, sizeof(txbuf));

    struct spi_ioc_transfer xfer[2];
    memset(xfer, 0, sizeof(xfer));

    txbuf[0] = 0x69;
    txbuf[1] = command;
    xfer[0].tx_buf = (unsigned long)txbuf;
    xfer[0].rx_buf = (unsigned long)data;
    xfer[0].len = 2; /* Length of  command to write*/
    xfer[0].cs_change = 0; /* Keep CS activated */
    xfer[0].delay_usecs = 0, //delay in us
    xfer[0].speed_hz = 2500000, //speed
    xfer[0].bits_per_word = 8, // bites per word 8

    xfer[1].rx_buf = (unsigned long)data;
    xfer[1].len = length; /* Length of Data to read */
    xfer[1].cs_change = 0; /* Keep CS activated */
    xfer[1].delay_usecs = 0;
    xfer[1].speed_hz = 2500000;
    xfer[1].bits_per_word = 8;

    int status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
    if (status < 0)
    {
        perror("SPI_IOC_MESSAGE");
        return -1;
    }

    printf("%x %x\n", txbuf[0], txbuf[1]);

    return 0;
}



int main(int argc, char **argv)
{
    int opt;
    char *devname="/dev/spidev1.0";

    while((opt = getopt(argc, argv, "p:")) != -1)
    {
        switch(opt)
        {
            case 'p':
                devname = strdup(optarg);
                break;
        }
    }

    int fd =  spi_init(devname);
    printf("Opened %s: %d\n", devname, fd);
    if(fd>=0)
    {
        char version_string[13];
        version_string[12] = 0;
        if(yost_read(fd, 223, 16, (uint8_t *)version_string) > 0)
        {
            printf("Software version: ");
            for(int i=0;i<12;i++)
            {
                printf("%x%s", version_string[i], i==11?"\n":" ");
            }
        }
    }



    if(fd >= 0)
        close(fd);
    return 0;
}
