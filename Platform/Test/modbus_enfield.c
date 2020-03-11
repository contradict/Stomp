#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
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
    if(configure_modbus_context(ctx, 1000000))
    {
        exit(1);
    }

    modbus_set_slave(ctx, 0x55);

    modbus_flush(ctx);

    uint16_t serial_number[2];

    for(uint16_t base = 0x100; base<0x400; base += 0x100)
    {
        printf("base: 0x%x\n", base);
        bzero(serial_number, sizeof(serial_number));
        if(modbus_read_input_registers(ctx, base + 4, 2, serial_number) == -1)
            printf("  Read serial number failed: %s\n", modbus_strerror(errno));
        else
            printf("  Serial Number: %d\n", *(uint32_t *)serial_number);

        /*
        usleep(500000);

        uint16_t value1 = 0, value2 = 0;

        modbus_read_registers(ctx, base + 14, 1, &value1);
        printf("  Initial Command: %d\n", value1);
        value2 = value1 + 1;
        modbus_write_registers(ctx, base + 14, 1, &value2);
        modbus_read_registers(ctx, base + 14, 1, &value1);
        printf("  Updated Command: %d\n", value1);
        value1 -= 1;
        modbus_write_registers(ctx, base + 14, 1, &value1);
        modbus_read_registers(ctx, base + 14, 1, &value2);
        printf("  Restored Command: %d\n", value2);

        if(modbus_read_registers(ctx, base + 11, 1, &value1) == -1)
            printf("  Read dither failed: %s\n", modbus_strerror(errno));
        else
            printf("  Initial Dither: %d\n", value1);
        value2 = value1 + 1;
        if(modbus_write_registers(ctx, base + 11, 1, &value2) == -1)
            printf("  Write dither failed: %s\n", modbus_strerror(errno));
        if(modbus_read_registers(ctx, base + 11, 1, &value1) == -1)
            printf("  Re-read dither failed: %s\n", modbus_strerror(errno));
        else
            printf("  Updated Dither: %d\n", value1);
        value1 -= 1;
        if(modbus_write_registers(ctx, base + 11, 1, &value1) == -1)
            printf("  Re-Write dither failed: %s\n", modbus_strerror(errno));
        if(modbus_read_registers(ctx, base + 11, 1, &value2) == -1)
            printf("  Re-Re-Read dither failed: %s\n", modbus_strerror(errno));
        else
            printf("  Restored Dither: %d\n", value2);
        */
    }


    modbus_close(ctx);

    return 0;
}
