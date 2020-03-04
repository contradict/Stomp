#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
/* termios2 */
#include <asm/ioctls.h>
#include <asm/termbits.h>
#include <linux/serial.h>

#include <modbus.h>

int main(int argc, char **argv)
{
    modbus_t *ctx;

    ctx = modbus_new_rtu("/dev/ttyS4", 9600, 'N', 8, 1);
    if(!ctx)
    {
        perror("Create modbus context");
        exit(1);
    }
    modbus_set_debug(ctx, true);

    struct timeval rto;
    rto.tv_sec = 0;
    rto.tv_usec = 20;
    modbus_set_byte_timeout(ctx, &rto);

    modbus_connect(ctx);

    struct termios2 tio;
    int fd = modbus_get_socket(ctx);
    ioctl(fd, TCGETS2, &tio);
    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_ospeed = 1000000;
    if(ioctl(fd, TCSETS2, &tio) < 0)
    {
        perror("Set speed failed");
        exit(1);
    }

    struct serial_rs485 rs485conf;
    /* Enable RS485 mode: */
    rs485conf.flags |= SER_RS485_ENABLED;
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
        exit(1);
    }

    modbus_set_slave(ctx, 0x55);

    modbus_flush(ctx);

    uint16_t value;
    uint8_t bits_value;
    for(int i=0;i<1;i++)
    {
        modbus_read_registers(ctx, 0x55, 1, &value);
        printf("r=%04x\n", value);
        modbus_read_bits(ctx, 0x55, 1, &bits_value);
        printf("c=%04x\n", bits_value);
        modbus_read_input_bits(ctx, 0x55, 1, &bits_value);
        printf("d=%04x\n", bits_value);
        modbus_read_input_registers(ctx, 0x55, 1, &value);
        printf("i=%04x\n", value);
        modbus_write_bit(ctx, 0x55, 0);
        modbus_read_bits(ctx, 0x55, 1, &bits_value);
        printf("c=%04x\n", bits_value);
        modbus_write_register(ctx, 0x55, 0x55AA);
        modbus_read_registers(ctx, 0x55, 1, &value);
        printf("r=%04x\n", value);
        bits_value = 0x0001;
        modbus_write_bits(ctx, 0x55, 1, &bits_value);
        modbus_read_bits(ctx, 0x55, 1, &bits_value);
        printf("c=%04x\n", bits_value);
        value = 0xAA55;
        modbus_write_registers(ctx, 0x55, 1, &value);
        modbus_read_registers(ctx, 0x55, 1, &value);
        printf("r=%04x\n", value);
        modbus_flush(ctx);
        usleep(500000);
    }

    modbus_close(ctx);

    return 0;
}
