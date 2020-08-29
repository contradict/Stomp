#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <modbus.h>
#include "modbus_device.h"

int main(int argc, char **argv)
{
    modbus_t *ctx;

    // Phony baud
    ctx = modbus_new_rtu("/dev/ttyS4", 9600, 'N', 8, 1);
    if(!ctx)
    {
        perror("Create modbus context");
        exit(1);
    }
    modbus_set_debug(ctx, true);

    // Actual baud rate here
    if(configure_modbus_context(ctx, 1000000, 50, 100000))
    {
        exit(1);
    }

    modbus_set_slave(ctx, 0x55);

    modbus_flush(ctx);

    uint16_t value;
    uint8_t bits_value;
    for(int i=0;i<8;i++)
    {
        modbus_read_registers(ctx, 0x55, 1, &value);
        printf("r=%04x\n", value);
        /*
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
        */
        modbus_flush(ctx);
        usleep(500000);
    }

    modbus_close(ctx);

    return 0;
}
