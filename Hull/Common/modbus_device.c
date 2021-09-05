#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>
/* termios2 */
#include <asm/ioctls.h>
#include <asm/termbits.h>
#include <linux/serial.h>
#include "modbus_device.h"


int configure_modbus_context(modbus_t *ctx, uint32_t custom_baud, uint32_t byte_timeout, uint32_t response_timeout)
{
    modbus_set_byte_timeout(ctx, 0, byte_timeout);

    // This is weird. The device responds in ~2ms with a
    // full round-trip to the servo. Local port latency?
    if(modbus_set_response_timeout(ctx, 0, response_timeout))
    {
        printf("Unable to set response timeout: %s\n", modbus_strerror(errno));
        return -1;
    }

    if(modbus_connect(ctx))
    {
        printf("Unable to connect: %s\n", modbus_strerror(errno));
        return -1;
    }

    struct termios2 tio;
    int fd = modbus_get_socket(ctx);
    if(fd<0)
    {
        printf("Invalid file descriptor from modbus_get_socket.\n");
        return -1;
    }
    ioctl(fd, TCGETS2, &tio);
    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_ospeed = custom_baud;
    if(ioctl(fd, TCSETS2, &tio) < 0)
    {
        perror("Set speed failed");
        return -1;
    }

    struct serial_rs485 rs485conf;
    /* Enable RS485 mode: */
    rs485conf.flags = SER_RS485_ENABLED;
    /* Set logical level for RTS pin equal to 0 when sending: */
    rs485conf.flags &= ~(SER_RS485_RTS_ON_SEND);
    /* Set logical level for RTS pin equal to 1 after sending: */
    rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;
    /* Set rts delay before send, if needed: */
    rs485conf.delay_rts_before_send = 0;
    /* Set rts delay after send, if needed: */
    rs485conf.delay_rts_after_send = 0;
    /* Set this flag if you want to receive data even while sending data */
    //rs485conf.flags |= SER_RS485_RX_DURING_TX;

    if (ioctl (fd, TIOCSRS485, &rs485conf) < 0) {
        perror("Configure RS485 failed");
        return -2;
    }
    return 0;
}

int create_modbus_interface(char *devname, uint32_t custom_baud, uint32_t byte_timeout, uint32_t response_timeout, modbus_t **ctx)
{
    // Phony baud
    *ctx = modbus_new_rtu(devname, 9600, 'N', 8, 1);
    if(!ctx)
    {
        perror("Create modbus context");
        return -1;
    }

    // Actual baud rate here
    if(configure_modbus_context(*ctx, custom_baud, byte_timeout, response_timeout))
    {
        return -1;
    }

    modbus_flush(*ctx);

    return 0;
}
