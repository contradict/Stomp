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
    clear();
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
        move(4 + 2 * j, 4);
        clrtoeol();
        addstr(display);
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
    bool iserror[3];
    char input_string[256];
    int input_pos;
    bool input_search;
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
    sc->input_pos = 0;
    sc->input_string[0] = 0;
    sc->input_search = false;
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
    clear();
}

void showerror(struct ServoContext *sc, const char *msg, int err)
{
    if(err == -1)
    {
        snprintf(sc->error[sc->joint], 256, "%s: %s", msg, modbus_strerror(errno));
        sc->iserror[sc->joint] = true;
    }
}

void servo_display(modbus_t *mctx, void *sctx)
{
    struct ServoContext *sc = (struct ServoContext *)sctx;
    char display[256];
    int16_t data[1];
    int err;
    for(int j=0;j<3;j++)
    {
        if(!sc->isinit[j])
        {
            move(4 + 2 * j + 0, 2);
            clrtoeol();
            if(sc->joint == j)
                attron(A_BOLD);
            addstr(joint_name[j]);
            attroff(A_BOLD);
            move(4 + 2 * j + 0, 15);
            addstr(sc->error[j]);
            continue;
        }
        err = modbus_read_input_registers(mctx, joint[j]+IFeedbackPosition, 1, data);
        if(!sc->iserror[j])
            showerror(sc, "Read feedback failed", err);
        if(err != -1)
        {
            sc->measured[j] = data[0];
        }
        move(4 + 4 * j + 0, 2);
        clrtoeol();
        if(sc->joint == j)
            attron(A_BOLD);
        addstr(joint_name[j]);
        attroff(A_BOLD);
        if(j == sc->joint && sc->input_pos>0)
        {
            move(4 + 4 * j + 0, 2 + 6);
            addstr(sc->input_string);
        }
        if(sc->iserror[j])
        {
            move(4 + 4 * j + 0, 2 + 15);
            addstr(sc->error[j]);
        }
        move(4 + 4 * j + 1, 2);
        clrtoeol();
        for(int p=0;p<7;p++)
        {
            move(4 + 4 * j + 1, 2 + 10 * p); 
            if(sc->joint == j && sc->param == p)
            {
                attron(A_BOLD);
            }
            printw("%s:", param_names[p]);
            if(sc->joint == j && sc->param == p)
            {
                attroff(A_BOLD);
            }
            printw(param_formats[p], sc->params[j][p] / param_scale[p]);
        }
        move(4 + 4 * j + 2, 2);
        clrtoeol();
        for(int p=7;p<13;p++)
        {
            move(4 + 4 * j + 2, 2 + 10 * (p - 7));
            if(sc->joint == j && sc->param == p)
            {
                attron(A_BOLD);
            }
            printw("%s:", param_names[p]);
            if(sc->joint == j && sc->param == p)
            {
                attroff(A_BOLD);
            }
            printw(param_formats[p], sc->params[j][p] / param_scale[p]);
        }
        move(4 + 4 * j + 2, 2 + 10 * 13);
        if(sc->joint == j && sc->param == 13)
        { 
            attron(A_BOLD);
        }
        printw(" c: ");
        if(sc->joint == j && sc->param == 13)
        {
            attroff(A_BOLD);
        }
        printw("0x%04x", sc->command[j]);
        move(4 + 4 * j + 3, 2 + 6*10);
        clrtoeol();
        printw("m: 0x%04x", sc->measured[j]);
        usleep(1000);
    }
    if(sc->param < 7)
        move(4 + 4*sc->joint + 1, 2 + 10*sc->param);
    else
        move(4 + 4*sc->joint + 2, 2 + 10*(sc->param - 7));
}

void servo_handle_key(modbus_t *mctx, void *sctx, int ch)
{
    struct ServoContext *sc = (struct ServoContext *)sctx;
    float input_value;
    int16_t register_value;
    char *endptr;
    int err;
    if(sc->input_search)
    {
        switch(ch)
        {
            case '\x0a':
                for(int p=0;p<13;p++)
                {
                    if(strstr(param_names[p], sc->input_string))
                    {
                        sc->param = p;
                        break;
                    }
                }
                sc->input_search = false;
                sc->input_pos = 0;
                sc->input_string[sc->input_pos] = 0;
                break;

            case KEY_BACKSPACE:
                sc->input_pos = sc->input_pos == 0 ? 0 : sc->input_pos - 1;
                sc->input_string[sc->input_pos] = 0;
                break;

            default:
                sc->input_string[sc->input_pos++] = ch;
                sc->input_string[sc->input_pos] = 0;
                if(sc->input_pos >= 255)
                {
                    sc->input_pos = 0;
                    sc->input_string[sc->input_pos] = 0;
                }
                break;
        }
        return;
    }
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
            err = modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand, sc->command[sc->joint]);
            showerror(sc, "Set command failed", err);
            break;
        case '>':
            err = modbus_write_bit(mctx, joint[sc->joint] + CSaveConfiguration, 0);
            showerror(sc, "Save failed", err);
            break;
        case '\x09': // TAB
            sc->joint = (sc->joint + 1) % 3;
            break;
        case KEY_BTAB:
            sc->joint = (sc->joint - 1) % 3;
            if(sc->joint < 0) sc->joint += 3;
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
                err = modbus_write_register(mctx, joint[sc->joint] + HProportionalGain + sc->param,
                        sc->params[sc->joint][sc->param]);
                showerror(sc, "Increase param failed", err);
            }
            else
            {
                sc->command[sc->joint] += 1;
                if(sc->command[sc->joint] > 4095)
                    sc->command[sc->joint] = 4095;
                err = modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                        sc->command[sc->joint]);
                showerror(sc, "Increase command failed", err);
            }
            break;
        case 'W':
            if(sc->param < 13)
            {
                sc->params[sc->joint][sc->param] += 10;
                if(sc->params[sc->joint][sc->param] > param_max[sc->param])
                    sc->params[sc->joint][sc->param] = param_max[sc->param];
                err = modbus_write_register(mctx, joint[sc->joint] + HProportionalGain + sc->param,
                        sc->params[sc->joint][sc->param]);
                showerror(sc, "Increase param failed", err);
            }
            else
            {
                sc->command[sc->joint] += 100;
                if(sc->command[sc->joint] > 4095)
                    sc->command[sc->joint] = 4095;
                err = modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                        sc->command[sc->joint]);
                showerror(sc, "Increase command failed", err);
            }
            break;
        case 'a':
            if(sc->param < 13)
            {
                sc->params[sc->joint][sc->param] -= 1;
                if(sc->params[sc->joint][sc->param] < 0)
                    sc->params[sc->joint][sc->param] = 0;
                err = modbus_write_register(mctx, joint[sc->joint] + HProportionalGain + sc->param,
                        sc->params[sc->joint][sc->param]);
                showerror(sc, "decrease param failed", err);
            }
            else
            {
                sc->command[sc->joint] -= 1;
                if(sc->command[sc->joint] < 0)
                    sc->command[sc->joint] = 0;
                err = modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                        sc->command[sc->joint]);
                showerror(sc, "decrease command failed", err);
            }
            break;
        case 'A':
            if(sc->param < 13)
            {
                sc->params[sc->joint][sc->param] -= 10;
                if(sc->params[sc->joint][sc->param] < 0)
                    sc->params[sc->joint][sc->param] = 0;
                err = modbus_write_register(mctx, joint[sc->joint] + HProportionalGain + sc->param,
                        sc->params[sc->joint][sc->param]);
                showerror(sc, "decrease param failed", err);
            }
            else
            {
                sc->command[sc->joint] -= 100;
                if(sc->command[sc->joint] < 0)
                    sc->command[sc->joint] = 0;
                err = modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                        sc->command[sc->joint]);
                showerror(sc, "decrease command failed", err);
            }
            break;
        case KEY_F(12):
            sc->iserror[sc->joint] = false;
            break;
        case 'x':
        case '.':
        case '0' ... '9':
            sc->input_string[sc->input_pos++] = ch;
            sc->input_string[sc->input_pos] = 0;
            if(sc->input_pos >= 255)
            {
                sc->input_pos = 0;
                sc->input_string[sc->input_pos] = 0;
            }
            break;
        case '/':
            sc->input_search = true;
            break;

        case KEY_BACKSPACE:
            sc->input_pos = sc->input_pos == 0 ? 0 : sc->input_pos - 1;
            sc->input_string[sc->input_pos] = 0;
            break;

        case '\x0a': // ENTER
            input_value = strtod(sc->input_string, &endptr);
            if(endptr != 0)
            {
                if(sc->param<13)
                {
                    register_value = input_value * param_scale[sc->param];
                    if(register_value<0)
                        register_value = 0;
                    if(register_value>param_max[sc->param])
                        register_value = param_max[sc->param];
                    sc->params[sc->joint][sc->param] = register_value;
                    err = modbus_write_register(mctx, joint[sc->joint] + HProportionalGain + sc->param,
                            sc->params[sc->joint][sc->param]);
                    showerror(sc, "set param failed", err);
                }
                else
                {
                    register_value = input_value;
                    if(register_value<0)
                        register_value = 0;
                    if(register_value>4095)
                        register_value = 4095;
                    sc->command[sc->joint] = register_value;
                    err = modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                            sc->command[sc->joint]);
                    showerror(sc, "set command failed", err);
                }
            }
            sc->input_pos = 0;
            sc->input_string[sc->input_pos] = 0;
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
        move(0, 30);
        clrtoeol();
        printw("'%s':(%d)", keyname(pch), pch);
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
