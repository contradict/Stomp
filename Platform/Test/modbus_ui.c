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
            snprintf(display, 256, "%s  s: %5.3fV a: %06.1fd f: %06.3fV l: %06.3fin b: %04x r: %04x",
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
    int16_t params[3][13];
    int16_t measured[3];
    int16_t command[3];
    int joint;
    int param;
    bool isinit[3];
    char error[3][256];
};

const char *param_names[13] = {
       " P",    " D",   " F",     " O",    " A",    " C",    "MN",    "MX",
       "RU",    "RD",   "DB",     "DI",    " V"};
const char *param_formats[13] = {
    "%6.1f", "%6.1f", "%6.1f", "%6.1f", "%6.4f", "%6.4f", "%6.1f", "%6.1f",
    "%6.1f", "%6.1f", "%6.1f", "%6.1f", "%6.1f"};
const float param_scale[13] = {
      10.0f,   10.0f,   10.0f,   10.0f,32767.0f, 1024.0f,   10.0f,   10.0f,
      10.0f,   10.0f,   10.0f,   10.0f,   10.0f};
const int16_t param_max[13] = {
       1000,   1000,     1000,    1000,   32767,   32767,    1000,    1000,
       1000,   1000,     1000,    1000,    1000};

void servo_init(modbus_t *mctx, void *sctx)
{
    struct ServoContext *sc = (struct ServoContext *)sctx;
    char display[256];
    int16_t data[1];
    sc->joint = 0;
    sc->param = 0;
    int n;
    for(int j=0;j<3;j++)
    {
        sc->isinit[j] = true;
        n = 0;
        if(modbus_read_registers(mctx, joint[j]+HProportionalGain, 13, sc->params[j]) == -1)
        {
            n = snprintf(sc->error[j], 256,
                    "%s: Gain read failed: %s %04x",
                    joint_name[j], modbus_strerror(errno), joint[j] + HProportionalGain);
            sc->isinit[j] = false;
        }
        if(modbus_read_registers(mctx, joint[j]+HCachedDigitalCommand, 1, data) == -1)
        {
            n += snprintf(sc->error[j] + n, 256 - n,
                    "Command read failed: %s", modbus_strerror(errno));
            sc->isinit[j] = false;
        }
        else
        {
            sc->command[j] = data[0];
        }
        usleep(50000);
    }
}

void servo_display(modbus_t *mctx, void *sctx)
{
    struct ServoContext *sc = (struct ServoContext *)sctx;
    char display[256];
    int16_t data[1];
    int n;
    for(int j=0;j<3;j++)
    {
        if(!sc->isinit[j])
        {
            if(sc->joint == j)
                attron(A_BOLD);
            mvaddstr(4 + 4 * j + 0, 2, sc->error[j]);
            refresh();
            attroff(A_BOLD);
            continue;
        }
        if(modbus_read_input_registers(mctx, joint[j]+IFeedbackPosition, 1, data) == -1)
        {
            snprintf(display, 256, "%s: Feedback read failed: %s",
                     joint_name[j], modbus_strerror(errno));
            mvaddstr(4 + 4 * j + 0, 2, sc->error[j]);
            refresh();
        }
        else
        {
            if(sc->joint == j)
                attron(A_BOLD);
            mvaddstr(4 + 4 * j + 0, 2, joint_name[j]);
            refresh();
            attroff(A_BOLD);
            sc->measured[j] = data[0];
        }
        n = 0;
        for(int p=0;p<7;p++)
        {
            n += snprintf(display + n, 256 - n, "%s:", param_names[p]);
            n += snprintf(display + n, 256 - n, param_formats[p],
                          sc->params[j][p] / param_scale[p]);
            display[n++] = ' ';
            display[n] = 0;
        }
        mvaddstr(4 + 4 * j + 1, 2, display);
        refresh();
        n = 0;
        for(int p=7;p<13;p++)
        {
            n += snprintf(display + n, 256 - n, "%s:", param_names[p]);
            n += snprintf(display + n, 256 - n, param_formats[p],
                          sc->params[j][p] / param_scale[p]);
            display[n++] = ' ';
            display[n] = 0;
        }
        snprintf(display + n, 256 - n, "c: 0x%04x", sc->command[j]);
        mvaddstr(4 + 4 * j + 2, 2, display);
        refresh();
        mvprintw(4 + 4 * j + 3, 2 + n, "m: 0x%04x", sc->measured[j]);
        refresh();
        usleep(1000);
    }
    if(sc->param < 7)
        move(4 + 4*sc->joint + 1, 2 + 10*sc->param);
    else
        move(4 + 4*sc->joint + 2, 2 + 10*(sc->param - 7));
}

//            sc->pgain[sc->joint] += 1;
//            sc->pgain[sc->joint] = sc->pgain[sc->joint]>1000 ? 1000 : sc->pgain[sc->joint];
//            modbus_write_register(mctx, joint[sc->joint] + HProportionalGain, sc->pgain[sc->joint]);
//            sc->pgain[sc->joint] -= 1;
//            sc->pgain[sc->joint] = sc->pgain[sc->joint]< 0 ? 0 : sc->pgain[sc->joint]; 
//            modbus_write_register(mctx, joint[sc->joint] + HProportionalGain, sc->pgain[sc->joint]);
//            sc->command[sc->joint] -= 1;
//            sc->command[sc->joint] = sc->command[sc->joint] < 0 ? 0 : sc->command[sc->joint];
//            modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand, sc->command[sc->joint]);
//            sc->command[sc->joint] += 1;
//            sc->command[sc->joint] = sc->command[sc->joint] > 4095 ? 4095 : sc->command[sc->joint];
//            modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand, sc->command[sc->joint]);
void servo_handle_key(modbus_t *mctx, void *sctx, int ch)
{
    struct ServoContext *sc = (struct ServoContext *)sctx;
    switch(ch)
    {
        case KEY_LEFT:
            sc->param = (sc->param - 1) % 14;
            if(sc->param < 0) sc->param += 14;
            break;
        case KEY_RIGHT:
            sc->param = (sc->param + 1) % 14;
            break;
        case 'M':
            sc->command[sc->joint] = sc->measured[sc->joint];
            modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                                  sc->command[sc->joint]);
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
        case 'w':
            if(sc->param < 13)
            {
                sc->params[sc->joint][sc->param] += 1;
                if(sc->params[sc->joint][sc->param] > param_max[sc->param])
                    sc->params[sc->joint][sc->param] = param_max[sc->param];
                modbus_write_register(mctx, joint[sc->joint] + HProportionalGain + sc->param,
                                      sc->params[sc->joint][sc->param]);
            }
            else
            {
                sc->command[sc->joint] += 1;
                if(sc->command[sc->joint] > 4095)
                    sc->command[sc->joint] = 4095;
                modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                                      sc->command[sc->joint]);
            }
            break;
        case 'W':
            if(sc->param < 13)
            {
                sc->params[sc->joint][sc->param] += 10;
                if(sc->params[sc->joint][sc->param] > param_max[sc->param])
                    sc->params[sc->joint][sc->param] = param_max[sc->param];
                modbus_write_register(mctx, joint[sc->joint] + HProportionalGain + sc->param,
                                      sc->params[sc->joint][sc->param]);
            }
            else
            {
                sc->command[sc->joint] += 100;
                if(sc->command[sc->joint] > 4095)
                    sc->command[sc->joint] = 4095;
                modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                                      sc->command[sc->joint]);
            }
            break;
        case 'a':
            if(sc->param < 13)
            {
                sc->params[sc->joint][sc->param] -= 1;
                if(sc->params[sc->joint][sc->param] < 0)
                    sc->params[sc->joint][sc->param] = 0;
                modbus_write_register(mctx, joint[sc->joint] + HProportionalGain + sc->param,
                                      sc->params[sc->joint][sc->param]);
            }
            else
            {
                sc->command[sc->joint] -= 1;
                if(sc->command[sc->joint] < 0)
                    sc->command[sc->joint] = 0;
                modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                                      sc->command[sc->joint]);
            }
            break;
        case 'A':
            if(sc->param < 13)
            {
                sc->params[sc->joint][sc->param] -= 10;
                if(sc->params[sc->joint][sc->param] < 0)
                    sc->params[sc->joint][sc->param] = 0;
                modbus_write_register(mctx, joint[sc->joint] + HProportionalGain + sc->param,
                                      sc->params[sc->joint][sc->param]);
            }
            else
            {
                sc->command[sc->joint] -= 100;
                if(sc->command[sc->joint] < 0)
                    sc->command[sc->joint] = 0;
                modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                                      sc->command[sc->joint]);
            }
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
    start_color();
    cbreak();
    noecho();
    clear();
    nodelay(stdscr, true);
    keypad(stdscr, true);
    bool go=true;
    int ch, pch;
    enum UIModes mode = SENSOR;
    while(go)
    {
        ch = getch();
        if(ch != -1)
        {
            pch = ch;
        }
        mvprintw(0, 30, "'%s':(%d)", keyname(pch), pch);
        refresh();
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
                refresh();
                usleep(period*1000);
                clear();
                break;
            case '':
                go = false;
                break;
            case KEY_F(1):
                sensor_init(ctx, &sensor);
                mode = SENSOR;
                break;
            case KEY_F(2):
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
