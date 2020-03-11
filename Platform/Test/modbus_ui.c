#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <modbus.h>
#include <curses.h>
#include <math.h>
#include "modbus_device.h"

#define JOINT_CURL  0x100
#define JOINT_SWING 0x200
#define JOINT_LIFT  0x300

void read_fast(modbus_t *ctx, int joint, uint16_t *angle, uint16_t *feedback,
               uint16_t *rodendpressure, uint16_t *baseendpressure)
{
    uint16_t start = joint + 0;
    uint16_t data[4];
    modbus_read_input_registers(ctx, start, 4, data);
    *angle = data[0];
    *feedback = data[1];
    *rodendpressure = data[2];
    *baseendpressure = data[3];
}

int main(int argc, char **argv)
{
    
    char *devname = "/dev/ttyS4";
    uint32_t baud = 1000000;
    uint8_t address = 0x55;
    int period = 50;

    int opt, err;
    char *offset;
    while((opt = getopt(argc, argv, "p:b:a:t:")) != -1)
    {
        switch(opt)
        {
            case 'p':
                devname = strdup(optarg);
                break;
            case 'b':
                baud = atol(optarg);
                break;
            case 'a':
                offset = strchr(optarg, 'x');
                if(offset)
                {
                    sscanf(offset + 1, "%lx", &address);
                }
                else
                {
                    address = atoi(optarg);
                }
                printf("address: 0x%x\n", address);
                break;
            case 't':
                period = atoi(optarg);
                break;
        }
    }

    modbus_t *ctx;

    // Phony baud
    ctx = modbus_new_rtu(devname, 9600, 'N', 8, 1);
    if(!ctx)
    {
        perror("Create modbus context");
        exit(1);
    }

    // Actual baud rate here
    if(configure_modbus_context(ctx, baud))
    {
        exit(1);
    }

    modbus_set_slave(ctx, address);

    modbus_flush(ctx);

    int joint = JOINT_SWING;
    int16_t command;
    if(modbus_read_registers(ctx, joint + 13, 1, &command) == -1)
    {
        printf("Read initial command failed: %s\n", modbus_strerror(errno));
        exit(1);
    }

    initscr();
    timeout(period);
    cbreak();
    noecho();
    clear();
    keypad(stdscr, true);
    bool go=true;
    int ch;
    int16_t angle, feedback, pr, pb, measured;
    int16_t proportional=0, derivative=0;
    char display[256];
    while(go)
    {
        ch = getch();
        switch(ch)
        {
            case ERR:
                read_fast(ctx, joint, &angle, &feedback, &pr, &pb);
                modbus_read_input_registers(ctx, joint + 7, 1, &measured);
                snprintf(display, 256, "a: %06.1f f:%06.3f m:%04x, rod:%04x base:%04x", angle / 1000.0 * 180 / M_PI, feedback / 1000.0f, measured, pr, pb);
                mvaddstr(10, 10, display);
                snprintf(display, 256, "p:%04d  d:%04d", proportional, derivative);
                mvaddstr(11, 10, display);
                snprintf(display, 256, "c:%04x", command);
                mvaddstr(12, 10, display);
                break;
            case 'Q':
            case 'q':
                go = false;
                break;
            case KEY_UP:
                proportional += 1;
                proportional = proportional>1000 ? 1000 : proportional;
                modbus_write_register(ctx, joint + 0, proportional);
                break;
            case KEY_DOWN:
                proportional -= 1;
                proportional = proportional < 0 ? 0 : proportional; 
                modbus_write_register(ctx, joint + 0, proportional);
                break;
            case KEY_LEFT:
                command -= 1;
                command = command < 0 ? 0 : command;
                modbus_write_register(ctx, joint + 13, command);
                break;
            case KEY_RIGHT:
                command += 1;
                command = command > 4095 ? 4095 : command;
                modbus_write_register(ctx, joint + 13, command);
                break;
         }
    }

    endwin();

    return 0;
}
