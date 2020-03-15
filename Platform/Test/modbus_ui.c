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
    SERVO,
    POSITION,
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

struct ErrorContext
{
    char error[256];
    bool iserror;
};

void error_display(int y, int x, struct ErrorContext *ec)
{
    if(ec->iserror)
    {
        move(y, x);
        addstr(ec->error);
    }
}

struct InputContext
{
    char input_string[256];
    int input_pos;
    bool input_search;
    int field_count;
    const char **input_fields;
};

void input_display(int y, int x, struct InputContext *ic)
{
    if(ic->input_pos>0)
    {
        move(y, x);
        addstr(ic->input_string);
    }
}

int input_handle_key(struct InputContext *ic, int ch, float *v)
{
    char *endptr;
    int ret = 0;
    float input_value;
    if(ic->input_search)
    {
        switch(ch)
        {
            case '\x0a':
                for(int p=0;p<ic->field_count;p++)
                {
                    if(strstr(ic->input_fields[p], ic->input_string))
                    {
                        *v = p;
                        ret = -2;
                    }
                }
                ic->input_search = false;
                ic->input_pos = 0;
                ic->input_string[ic->input_pos] = 0;
                break;

            case KEY_BACKSPACE:
                ic->input_pos = ic->input_pos == 0 ? 0 : ic->input_pos - 1;
                ic->input_string[ic->input_pos] = 0;
                break;

            default:
                ic->input_string[ic->input_pos++] = ch;
                ic->input_string[ic->input_pos] = 0;
                if(ic->input_pos >= 255)
                {
                    ic->input_pos = 0;
                    ic->input_string[ic->input_pos] = 0;
                }
                break;
        }
        ret = -1;
    }
    else
    {
        switch(ch)
        {
            case 'x':
            case '.':
            case '0' ... '9':
                ic->input_string[ic->input_pos++] = ch;
                ic->input_string[ic->input_pos] = 0;
                if(ic->input_pos >= 255)
                {
                    ic->input_pos = 0;
                    ic->input_string[ic->input_pos] = 0;
                }
                break;
            case '/':
                ic->input_search = true;
                break;

            case KEY_BACKSPACE:
                ic->input_pos = ic->input_pos == 0 ? 0 : ic->input_pos - 1;
                ic->input_string[ic->input_pos] = 0;
                break;

            case '\x0a': // ENTER
                input_value = strtod(ic->input_string, &endptr);
                if(endptr != 0)
                {
                    *v = input_value;
                    ret = -3;
                }
                ic->input_pos = 0;
                ic->input_string[ic->input_pos] = 0;
                break;
        }
    }
    return ret;
}

struct ServoContext
{
    int16_t params[3][13];
    int16_t measured[3];
    int16_t command[3];
    int joint;
    int param;
    bool isinit[3];
    struct ErrorContext error[3];
    struct InputContext input;
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
    sc->input.input_pos = 0;
    sc->input.input_string[0] = 0;
    sc->input.input_search = false;
    sc->input.input_fields = param_names;
    sc->input.field_count = 13;
    int n;
    for(int j=0;j<3;j++)
    {
        sc->isinit[j] = true;
        n = 0;
        if(modbus_read_registers(mctx, joint[j]+HProportionalGain, 13, sc->params[j]) == -1)
        {
            n = snprintf(sc->error[j].error, 256,
                    "%s: Gain read failed: %s %04x",
                    joint_name[j], modbus_strerror(errno), joint[j] + HProportionalGain);
            sc->isinit[j] = false;
        }
        if(modbus_read_registers(mctx, joint[j]+HCachedDigitalCommand, 1, data) == -1)
        {
            n += snprintf(sc->error[j].error + n, 256 - n,
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

void showerror(struct ErrorContext *ec, const char *msg, int err)
{
    if(err == -1 && !ec->iserror)
    {
        snprintf(ec->error, 256, "%s: %s", msg, modbus_strerror(errno));
        ec->iserror = true;
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
            addstr(sc->error[j].error);
            continue;
        }
        err = modbus_read_input_registers(mctx, joint[j]+IFeedbackPosition, 1, data);
        showerror(&sc->error[j], "Read feedback failed", err);
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
        if(j == sc->joint)
            input_display(4 + 4 * j + 0, 2 + 6, &sc->input);
        error_display(4 + 4 * j + 0, 2 + 15, &sc->error[j]);
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
    int err;
    int handle;
    handle = input_handle_key(&sc->input, ch, &input_value);
    if(handle)
    {
        switch(handle)
        {
            case -3:
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
                    showerror(&sc->error[sc->joint], "set param failed", err);
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
                    showerror(&sc->error[sc->joint], "set command failed", err);
                }
                break;
            case -2:
                sc->param = input_value;
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
            showerror(&sc->error[sc->joint], "Set command failed", err);
            break;
        case '>':
            err = modbus_write_bit(mctx, joint[sc->joint] + CSaveConfiguration, 0);
            showerror(&sc->error[sc->joint], "Save failed", err);
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
                showerror(&sc->error[sc->joint], "Increase param failed", err);
            }
            else
            {
                sc->command[sc->joint] += 1;
                if(sc->command[sc->joint] > 4095)
                    sc->command[sc->joint] = 4095;
                err = modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                        sc->command[sc->joint]);
                showerror(&sc->error[sc->joint], "Increase command failed", err);
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
                showerror(&sc->error[sc->joint], "Increase param failed", err);
            }
            else
            {
                sc->command[sc->joint] += 100;
                if(sc->command[sc->joint] > 4095)
                    sc->command[sc->joint] = 4095;
                err = modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                        sc->command[sc->joint]);
                showerror(&sc->error[sc->joint], "Increase command failed", err);
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
                showerror(&sc->error[sc->joint], "decrease param failed", err);
            }
            else
            {
                sc->command[sc->joint] -= 1;
                if(sc->command[sc->joint] < 0)
                    sc->command[sc->joint] = 0;
                err = modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                        sc->command[sc->joint]);
                showerror(&sc->error[sc->joint], "decrease command failed", err);
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
                showerror(&sc->error[sc->joint], "decrease param failed", err);
            }
            else
            {
                sc->command[sc->joint] -= 100;
                if(sc->command[sc->joint] < 0)
                    sc->command[sc->joint] = 0;
                err = modbus_write_register(mctx, joint[sc->joint] + HCachedDigitalCommand,
                        sc->command[sc->joint]);
                showerror(&sc->error[sc->joint], "decrease command failed", err);
            }
            break;
        case KEY_F(12):
            sc->error[sc->joint].iserror = false;
            break;
    }
}

struct PositionContext {
    float step;
    int coordinate;
    float position_display[3];
    float position_command[3];
    struct ErrorContext error;
    struct InputContext input;
};

const char *coordinate_names[3] = {"x", "y", "z"};

void position_init(modbus_t *mctx, void *pctx)
{
    struct PositionContext *pc = (struct PositionContext *)pctx;
    int16_t pos[3];
    int err;
    pc->step = 0.100;
    pc->coordinate = 0;
    pc->input.input_pos = 0;
    pc->input.input_string[0] = 0;
    pc->input.input_search = false;
    pc->input.input_fields = coordinate_names;
    pc->input.field_count = 3;
    err = modbus_read_registers(mctx, 0x40, 3, pos);
    showerror(&pc->error, "Read initial position failed", err);
    if(err != -1)
    {
        for(int i=0;i<3;i++)
        {
            pc->position_display[i] = pos[i] / 1000.0f;
            pc->position_command[i] = pos[i] / 1000.0f;
        }
    }
    clear();
}

void position_display(modbus_t *mctx, void *pctx)
{
    struct PositionContext *pc = (struct PositionContext *)pctx;
    int err;
    int16_t pos[3];

    err = modbus_read_registers(mctx, 0x40, 3, pos);
    showerror(&pc->error, "Read position failed", err);
    if(err != -1)
    {
        for(int i=0;i<3;i++)
            pc->position_display[i] = pos[i] / 1000.0f;
    }
 
    move(5, 2);
    clrtoeol();
    error_display(5, 2, &pc->error);

    move(7, 2);
    clrtoeol();
    printw("X: %4.2f Y: %4.2f Z: %4.2f",
           pc->position_display[0], pc->position_display[1], pc->position_display[2]);
    move(8, 2);
    clrtoeol();
    printw("X: %4.2f Y: %4.2f Z: %4.2f",
           pc->position_command[0], pc->position_command[1], pc->position_command[2]);
}

void position_go(modbus_t *mctx, struct PositionContext *pc)
{
    int16_t pos[3];
    int err;
    for(int c=0;c<3;c++)
        pos[c] = 1000.0f * pc->position_command[c];
    err = modbus_write_registers(mctx, 0x40, 3, pos);
    showerror(&pc->error, "Go position failed", err);
}

void position_handle_key(modbus_t *mctx, void *pctx, int ch)
{
    struct PositionContext *pc = (struct PositionContext *)pctx;
    int handle;
    float input_value;
    handle = input_handle_key(&pc->input, ch, &input_value);
    if(handle)
    {
        switch(handle)
        {
            case -2:
                pc->coordinate = input_value;
                break;
            case -3:
                pc->position_command[pc->coordinate] = input_value;
                break;
        }
        return;
    }
    switch(ch)
    {
        case KEY_F(12):
            pc->error.iserror = false;
            break;
        case 'w':
            pc->position_command[1] += pc->step;
            position_go(mctx, pc);
            break;
        case 'a':
            pc->position_command[0] -= pc->step;
            position_go(mctx, pc);
            break;
        case 's':
            pc->position_command[1] -= pc->step;
            position_go(mctx, pc);
            break;
        case 'd':
            pc->position_command[0] += pc->step;
            position_go(mctx, pc);
            break;
        case KEY_UP:
            pc->position_command[2] += pc->step;
            position_go(mctx, pc);
            break;
        case KEY_DOWN:
            pc->position_command[2] -= pc->step;
            position_go(mctx, pc);
            break;
        case 'G':
            position_go(mctx, pc);
            break;
        case 'M':
            memcpy(pc->position_command, pc->position_display, sizeof(pc->position_command));
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
    struct PositionContext position;

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
                    case POSITION:
                        position_display(ctx, &position);
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
            case KEY_F(3):
                position_init(ctx, &position);
                mode = POSITION;
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
                    case POSITION:
                        position_init(ctx, &position);
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
                    case POSITION:
                        position_handle_key(ctx, &position, ch);
                        break;
                }
        }
    }

    endwin();

    return 0;
}
