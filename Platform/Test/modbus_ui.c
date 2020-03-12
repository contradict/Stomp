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
#include "modbus_register_map.h"

enum UIModes {
    SENSOR,
    SERVO
};

static int joint[3] = {CURL_BASE, SWING_BASE, LIFT_BASE};
static char *joint_name[3] = {"Curl ", "Swing", "Lift "};

struct SensorContext {
};

void sensor_init(modbus_t *mctx, void *sctx)
{
}

void sensor_display(modbus_t *mctx, void *sctx)
{
    int err;
    char display[256];
    int16_t data[6];

    for(int j=0;j<3;j++)
    {
        if(modbus_read_input_registers(mctx, joint[j] + ISensorVoltage, 6, data) == -1)
        {
            snprintf(display, 256, "%s: Cached read failed: %s",
                     joint_name[j], modbus_strerror(errno));
        }
        else
        {
            snprintf(display, 256, "%s  s: %5.3fV a: %06.1fd l: %06.3fin f: %06.3fV b: %04x r: %04x",
                joint_name[j],
                data[0] / 1000.0f, data[1] / 1000.0f * 180.0f / M_PI,
                data[2] / 1000.0f, data[3] / 1000.0f,
                data[4], data[5]);
        }
        mvaddstr(4 + 2 * j, 4, display);
        refresh();
    }
}

void sensor_handle_key(modbus_t *mctx, void *sctx, int ch)
{
}

struct ServoContext
{
    int16_t pgain[3], dgain[3];
    int16_t measured[3];
    int16_t command[3];
    int joint;
};

void servo_init(modbus_t *mctx, void *sctx)
{
    struct ServoContext *sc = (struct ServoContext *)sctx;
    char display[256];
    int16_t data[2];
    int n;
    sc->joint = 0;
    for(int j=0;j<3;j++)
    {
        n = 0;
        if(modbus_read_registers(mctx, joint[j]+HProportionalGain, 2, data) == -1)
        {
            n = snprintf(display, 256, "%s: Gain read failed: %s",
                         joint_name[j], modbus_strerror(errno));
        }
        else
        {
            n = snprintf(display, 256, "%s  p: % 5.1f d: % f5.1f",
                         joint_name[j], data[0] / 10.0f, data[1] / 10.0f);
            sc->pgain[j] = data[0];
            sc->dgain[j] = data[1];
        }
        if(modbus_read_registers(mctx, joint[j]+HDigitalCommand, 1, data) == -1)
        {
            snprintf(display + n, 256-n, " Command read failed: %s",
                     modbus_strerror(errno));
        }
        else
        {
            snprintf(display + n, 256 - n, "c: 0x%04x", data[0]);
            sc->command[j] = data[0];
        }
        mvaddstr(4 + 2 * j, 4, display);
        usleep(50000);
    }
}

void servo_display(modbus_t *mctx, void *sctx)
{
    struct ServoContext *sc = (struct ServoContext *)sctx;
    char display[256];
    int16_t data[1];
    mvaddstr(3, 2, joint_name[sc->joint]);
    for(int j=0;j<3;j++)
    {
        if(modbus_read_input_registers(mctx, joint[j]+IFeedbackPosition, 1, data) == -1)
        {
            snprintf(display, 256, "%s: Feedback read failed: %s",
                     joint_name[j], modbus_strerror(errno));
        }
        else
        {
            snprintf(display, 256, "%s  p: % 5.1f d: % 5.1f m: 0x%04x c: 0x%04x",
                joint_name[j],
                sc->pgain[j] / 10.0f, sc->dgain[j] / 10.0f, data[0],
                sc->command[j]);
            sc->measured[j] = data[0];
        }
        mvaddstr(4 + 2 * j, 4, display);
        refresh();
        usleep(1000);
    }
}

void servo_handle_key(modbus_t *mctx, void *sctx, int ch)
{
    struct ServoContext *sc = (struct ServoContext *)sctx;
    switch(ch)
    {
        case KEY_UP:
            sc->pgain[sc->joint] += 1;
            sc->pgain[sc->joint] = sc->pgain[sc->joint]>1000 ? 1000 : sc->pgain[sc->joint];
            modbus_write_register(mctx, joint[sc->joint] + HProportionalGain, sc->pgain[sc->joint]);
            break;
        case KEY_DOWN:
            sc->pgain[sc->joint] -= 1;
            sc->pgain[sc->joint] = sc->pgain[sc->joint]< 0 ? 0 : sc->pgain[sc->joint]; 
            modbus_write_register(mctx, joint[sc->joint] + HProportionalGain, sc->pgain[sc->joint]);
            break;
        case KEY_LEFT:
            sc->command[sc->joint] -= 1;
            sc->command[sc->joint] = sc->command[sc->joint] < 0 ? 0 : sc->command[sc->joint];
            modbus_write_register(mctx, joint[sc->joint] + HDigitalCommand, sc->command[sc->joint]);
            break;
        case KEY_RIGHT:
            sc->command[sc->joint] += 1;
            sc->command[sc->joint] = sc->command[sc->joint] > 4095 ? 4095 : sc->command[sc->joint];
            modbus_write_register(mctx, joint[sc->joint] + HDigitalCommand, sc->command[sc->joint]);
            break;
        case 'M':
            sc->command[sc->joint] = sc->measured[sc->joint];
            modbus_write_register(mctx, joint[sc->joint] + HDigitalCommand, sc->command[sc->joint]);
            break;
        case 'C':
            sc->joint = 0;
            break;
        case 'S':
            sc->joint = 1;
            break;
        case 'L':
            sc->joint = 2;
            break;
    }
}

int main(int argc, char **argv)
{
    
    char *devname = "/dev/ttyS4";
    uint32_t baud = 1000000;
    uint8_t address = 0x55;
    int period = 100;

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
    if(configure_modbus_context(ctx, baud, 100000))
    {
        exit(1);
    }

    modbus_set_slave(ctx, address);

    modbus_flush(ctx);

    struct ServoContext servo;
    struct SensorContext sensor;

    initscr();
    cbreak();
    noecho();
    clear();
    nodelay(stdscr, true);
    keypad(stdscr, true);
    bool go=true;
    int ch;
    enum UIModes mode = SENSOR;
    while(go)
    {
        ch = getch();
        clear();
        switch(ch)
        {
            case ERR:
                switch(mode)
                {
                    case SENSOR:
                        sensor_display(ctx, &sensor);
                        break;
                    case SERVO:
                        servo_display(ctx, &servo);
                        break;
                }
                usleep(period*1000);
                break;
            case 'Q':
            case 'q':
                go = false;
                break;
            case 's':
                sensor_init(ctx, &sensor);
                mode = SENSOR;
                break;
            case 'v':
                servo_init(ctx, &servo);
                mode = SERVO;
                break;
            case 'I':
                switch(mode)
                {
                    case SENSOR:
                        sensor_init(ctx, &sensor);
                        break;
                    case SERVO:
                        servo_init(ctx, &servo);
                        break;
                }
                break;
            default:
                switch(mode)
                {
                    case SENSOR:
                        sensor_handle_key(ctx, &sensor, ch);
                        break;
                    case SERVO:
                        servo_handle_key(ctx, &servo, ch);
                        break;
                }
        }
    }

    endwin();

    return 0;
}
