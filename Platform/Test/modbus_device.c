#include <stdio.h>
#include <sys/ioctl.h>
/* termios2 */
#include <asm/ioctls.h>
#include <asm/termbits.h>
#include <linux/serial.h>
#include "modbus_device.h"


int configure_modbus_context(modbus_t *ctx, int custom_baud, int32_t response_timeout)
{
    struct timeval to;
    to.tv_sec = -1;
    to.tv_usec = 0; //2*10*1000000 / custom_baud;
    modbus_set_byte_timeout(ctx, &to);

    to.tv_sec = 0;
    to.tv_usec = response_timeout;
    // This is weird. The device responds in ~2ms with a
    // full round-trip to the servo. Local port latency?
    modbus_set_response_timeout(ctx, &to);

    modbus_connect(ctx);

    struct termios2 tio;
    int fd = modbus_get_socket(ctx);
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


