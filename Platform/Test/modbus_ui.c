#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <modbus.h>
#include <curses.h>
#include <math.h>
#include <sched.h>
#include <sys/time.h>

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

enum InputAction
{
    InputNoAction = 0,
    InputProcessed = -1,
    InputFoundParam = -2,
    InputValue = -3
};

enum InputAction input_handle_key(struct InputContext *ic, int ch, float *v)
{
    char *endptr;
    int ret = 0;
    float input_value;
    if(ic->input_search)
    {
        ret = -1;
        switch(ch)
        {
            case '\x0a':
                for(int p=0;p<ic->field_count;p++)
                {
                    if(strstr(ic->input_fields[p], ic->input_string))
                    {
                        *v = p;
                        ret = InputFoundParam;
                        break;
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
                input_value = strtof(ic->input_string, &endptr);
                if(endptr != 0)
                {
                    *v = input_value;
                    ret = InputValue;
                }
                ic->input_pos = 0;
                ic->input_string[ic->input_pos] = 0;
                break;
        }
    }
    return ret;
}

#define NUM_SERVO_PARAMS 14

struct ServoContext
{
    int16_t params[3][NUM_SERVO_PARAMS];
    int16_t measured[3];
    int joint;
    int param;
    int param_max;
    bool isinit[3];
    bool siggen_enable;
    bool siggen_start;
    float siggen_phase;
    float siggen_amplitude;
    float siggen_frequency;
    float siggen_signal;
    struct timeval last_siggen_output;
    struct ErrorContext error[3];
    struct InputContext input;
};

const char *param_names[NUM_SERVO_PARAMS + 2] = {
       " P",    " D",   " F",     " O",    " A",
       " C",    "MN",    "MX",
       "RU",    "RD",   "DB",     "DI",    " V", "CM", "sf", "sa"};
const char *param_formats[NUM_SERVO_PARAMS + 2] = {
    "%6.1f", "%6.1f", "%6.1f", "%6.1f", "%6.4f",
    "%6.4f", "%6.1f", "%6.1f", "%6.1f", "%6.1f",
    "%6.1f", "%6.1f", "%6.1f", "%6.2f", "%6.1f", "%6.1f"};
uint16_t param_register[NUM_SERVO_PARAMS] = {
    HProportionalGain, HDerivativeGain, HForceDamping, HOffset, HAreaRatio,
    HCylinderBore, HMinimumPosition, HMaximumPosition, HRampUp, HRampDown,
    HDeadBand, HDitherAmplitude, HValveOffset, HCachedDigitalCommand};
const float param_scale[NUM_SERVO_PARAMS] = {
      10.0f,   10.0f,   10.0f,   10.0f,32767.0f,
    1024.0f,   10.0f,   10.0f,   10.0f,   10.0f,
      10.0f,   10.0f,   10.0f,   40.95f};
const int16_t param_max[NUM_SERVO_PARAMS] = {
       1000,   1000,     1000,    1000,   32767,
      32767,    1000,    1000,    1000,    1000,
       1000,    1000,    1000,    4095};

void showerror(struct ErrorContext *ec, const char *msg, int err, struct timeval *dt)
{
    if(err == -1 && !ec->iserror)
    {
        if(dt)
        {
            snprintf(ec->error, 256, "%s(%d): %s", msg, dt->tv_usec, modbus_strerror(errno));
        }
        else
        {
            snprintf(ec->error, 256, "%s: %s", msg, modbus_strerror(errno));
        }
        ec->iserror = true;
    }
}

void servo_init(modbus_t *mctx, void *sctx)
{
    struct ServoContext *sc = (struct ServoContext *)sctx;
    char display[256];
    int16_t data[1];
    sc->joint = 0;
    sc->param = 0;
    sc->param_max = NUM_SERVO_PARAMS;
    sc->siggen_enable = false;
    sc->input.input_pos = 0;
    sc->input.input_string[0] = 0;
    sc->input.input_search = false;
    sc->input.input_fields = param_names;
    sc->input.field_count = NUM_SERVO_PARAMS;
    int n;
    int err;
    uint32_t sto_sec, sto_usec;
    modbus_get_response_timeout(mctx, &sto_sec, &sto_usec);
    modbus_set_response_timeout(mctx, 0, 100000);
    for(int j=0;j<3;j++)
    {
        sc->error[j].iserror = false;
        sc->error[j].error[0] = 0;
        sc->isinit[j] = true;
        n = 0;
        err = modbus_read_registers(
                mctx, joint[j]+HProportionalGain, 13, sc->params[j]);
        showerror(&sc->error[j], "Initial Gain Read failed", err, NULL);
        if(err == -1)
        {
            sc->isinit[j] = false;
        }
        err = modbus_read_registers(
                mctx, joint[j]+HCachedDigitalCommand, 1, data);
        showerror(&sc->error[j], "Initial Command Read failed", err, NULL);
        if(err != -1)
        {
            sc->params[j][13] = data[0];
        }
    }
    modbus_set_response_timeout(mctx, sto_sec, sto_usec);
    clear();
}

void start_signal_generator(struct ServoContext *sc)
{
    sc->param = 0;
    sc->param_max = NUM_SERVO_PARAMS + 2;
    sc->input.field_count = NUM_SERVO_PARAMS + 2;
    sc->siggen_phase = 0;
    sc->siggen_amplitude = 0;
    sc->siggen_frequency = 0;
    gettimeofday(&sc->last_siggen_output, NULL);
    sc->siggen_start = false;
    sc->siggen_enable = true;
}

void stop_signal_generator(struct ServoContext *sc)
{
    sc->param = 0;
    sc->param_max = NUM_SERVO_PARAMS;
    sc->input.field_count = NUM_SERVO_PARAMS;
    sc->siggen_enable = false;
}

void step_signal_generator(modbus_t *mctx, struct ServoContext *sc)
{
    struct timeval now, dt;
    int err;
    if(sc->siggen_enable)
    {
        gettimeofday(&now, NULL);
        timersub(&now, &sc->last_siggen_output, &dt);
        sc->siggen_phase += 2.0f*M_PI*sc->siggen_frequency * dt.tv_usec / 1.0e6f;
        sc->siggen_signal = 2047.0f + 40.95f * sc->siggen_amplitude * sinf(sc->siggen_phase);
        while(sc->siggen_phase > 2.0f * M_PI) sc->siggen_phase -= 2.0f * M_PI;
        if(sc->siggen_start || (fabs(sc->measured[sc->joint] - sc->siggen_signal) < 100))
        {
            sc->params[sc->joint][13] = sc->siggen_signal;
            err = modbus_write_register(
                mctx, joint[sc->joint] + HCachedDigitalCommand,
                sc->params[sc->joint][13]);
            showerror(&sc->error[sc->joint], "siggen set failed", err, NULL);
            sc->siggen_start = true;
        }
        memcpy(&sc->last_siggen_output, &now, sizeof(struct timeval));
    }
}

void servo_display(modbus_t *mctx, void *sctx)
{
    struct ServoContext *sc = (struct ServoContext *)sctx;
    char display[256];
    int16_t data[1];
    int err;
    struct timeval before, after;
    for(int j=0;j<3;j++)
    {
        err = modbus_read_input_registers(mctx, joint[j]+ICachedFeedbackPosition, 1, data);
        showerror(&sc->error[j], "Read feedback failed", err);
        if(err != -1)
        {
            sc->measured[j] = data[0];
            gettimeofday(&before, NULL);
            err = modbus_read_input_registers(mctx, joint[j]+ICachedFeedbackPosition, 1, data);
            gettimeofday(&after, NULL);
            timersub(&after, &before, &after);
            showerror(&sc->error[j], "Read feedback failed", err, &after);
        }
        step_signal_generator(mctx, sc);
        move(4 + 4 * j + 0, 2);
        clrtoeol();
        if(sc->joint == j)
            attron(A_BOLD);
        printw("%s(%c)", joint_name[j], sc->isinit[j]?'t':'f');
        attroff(A_BOLD);
        if(j == sc->joint)
            input_display(4 + 4 * j + 0, 2 + 9, &sc->input);
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
        for(int p=7;p<14;p++)
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
        move(4 + 4 * j + 3, 2);
        clrtoeol();
        if(sc->siggen_enable && j == sc->joint)
        {
            for(int p=NUM_SERVO_PARAMS;p<NUM_SERVO_PARAMS + 2; p++)
            {
                move(4 + 4 * j + 3, 2 + 10 * (p - NUM_SERVO_PARAMS));
                if(sc->param == p)
                {
                    attron(A_BOLD);
                }
                printw("%s:", param_names[p]);
                attroff(A_BOLD);
                switch(p-NUM_SERVO_PARAMS)
                {
                    case 0:
                        printw(param_formats[p], sc->siggen_frequency);
                        break;
                    case 1:
                        printw(param_formats[p], sc->siggen_amplitude);
                        break;
                }
            }
            move(4 + 4 * j + 3, 2 + 10 * 2);
            printw("ph:%6.1f", sc->siggen_phase);
            move(4 + 4 * j + 3, 2 + 10 * 3);
            printw("ss:%6.1f", sc->siggen_signal / 40.95);
            
        }
        move(4 + 4 * j + 3, 2 + 6*10);
        printw("m:%6.1f", sc->measured[j] / 40.95f);
    }
    if(sc->param < 7)
        move(4 + 4*sc->joint + 1, 2 + 10*sc->param);
    else if(sc->param < NUM_SERVO_PARAMS)
        move(4 + 4*sc->joint + 2, 2 + 10*(sc->param - 7));
    else
        move(4 + 4*sc->joint + 3, 2 + 10*(sc->param - NUM_SERVO_PARAMS));
}

#define CLIP(x, mn, mx) (((x)<(mn))?(mn):(((x)>(mx))?(mx):(x)))

void set_param(modbus_t *mctx, struct ServoContext *sc, float value)
{
    int err;
    if(sc->param<NUM_SERVO_PARAMS)
    {
        sc->params[sc->joint][sc->param] = value * param_scale[sc->param];
        sc->params[sc->joint][sc->param] = CLIP(
            sc->params[sc->joint][sc->param], 0, param_max[sc->param]);
        err = modbus_write_register(
                mctx, joint[sc->joint] + param_register[sc->param],
                sc->params[sc->joint][sc->param]);
        showerror(&sc->error[sc->joint], "change param failed", err, NULL);
    }
    else if(sc->siggen_enable)
    {
        switch(sc->param - NUM_SERVO_PARAMS)
        {
            case 0:
                sc->siggen_frequency = value;
                sc->siggen_frequency = CLIP(sc->siggen_frequency, 0.0f, 5.0f);
                break;
            case 1:
                sc->siggen_amplitude = value;
                sc->siggen_amplitude = CLIP(sc->siggen_amplitude, 0.0f, 100.0f);
                break;
        }
    }
}

void increment_param(modbus_t *mctx, struct ServoContext *sc, int increment)
{
    int err;
    if(sc->param<NUM_SERVO_PARAMS)
    {
        sc->params[sc->joint][sc->param] += increment;
        sc->params[sc->joint][sc->param] = CLIP(
            sc->params[sc->joint][sc->param], 0, param_max[sc->param]);
        err = modbus_write_register(
                mctx, joint[sc->joint] + param_register[sc->param],
                sc->params[sc->joint][sc->param]);
        showerror(&sc->error[sc->joint], "change param failed", err, NULL);
    }
    else if(sc->siggen_enable)
    {
        switch(sc->param - NUM_SERVO_PARAMS)
        {
            case 0:
                sc->siggen_frequency += increment / 10.0f;
                sc->siggen_frequency = CLIP(sc->siggen_frequency, 0.0f, 5.0f);
                break;
            case 1:
                sc->siggen_amplitude += increment / 10.0f;
                sc->siggen_amplitude = CLIP(sc->siggen_amplitude, 0.0f, 100.0f);
                break;
        }
    }
}

void servo_handle_key(modbus_t *mctx, void *sctx, int ch)
{
    struct ServoContext *sc = (struct ServoContext *)sctx;
    float input_value;
    int16_t register_value;
    int err;
    enum InputAction input;
    input = input_handle_key(&sc->input, ch, &input_value);
    switch(input)
    {
        case InputNoAction:
            break;
        case InputProcessed:
            return;
            break;
        case InputValue:
            set_param(mctx, sc, input_value);
            break;
        case InputFoundParam:
            sc->param = input_value;
            break;
    }
    switch(ch)
    {
        case KEY_LEFT:
            sc->param = (sc->param - 1);
            if(sc->param < 0) sc->param += sc->param_max;
            break;
        case KEY_RIGHT:
            sc->param = (sc->param + 1) % sc->param_max;
            break;
        case 'M':
            sc->params[sc->joint][13] = sc->measured[sc->joint];
            err = modbus_write_register(
                mctx, joint[sc->joint] + HCachedDigitalCommand,
                sc->params[sc->joint][13]);
            showerror(&sc->error[sc->joint], "Set command failed", err, NULL);
            break;
        case '>':
            err = modbus_write_bit(mctx, joint[sc->joint] + CSaveConfiguration, 0);
            showerror(&sc->error[sc->joint], "Save failed", err, NULL);
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
            increment_param(mctx, sc, 1);
            break;
        case 'W':
            increment_param(mctx, sc, 10);
            break;
        case 'a':
            increment_param(mctx, sc, -1);
            break;
        case 'A':
            increment_param(mctx, sc, -10);
            break;
        case 'G':
            start_signal_generator(sc);
            break;
        case 'g':
            stop_signal_generator(sc);
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
    pc->error.iserror = false;
    pc->error.error[0] = 0;
    pc->input.input_pos = 0;
    pc->input.input_string[0] = 0;
    pc->input.input_search = false;
    pc->input.input_fields = coordinate_names;
    pc->input.field_count = 3;
    err = modbus_read_registers(mctx, 0x40, 3, pos);
    showerror(&pc->error, "Read initial position failed", err, NULL);
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
    showerror(&pc->error, "Read position failed", err, NULL);
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
    showerror(&pc->error, "Go position failed", err, NULL);
}

void position_handle_key(modbus_t *mctx, void *pctx, int ch)
{
    struct PositionContext *pc = (struct PositionContext *)pctx;
    enum InputAction input;
    float input_value;
    input = input_handle_key(&pc->input, ch, &input_value);
    switch(input)
    {
        case InputNoAction:
            break;
        case InputProcessed:
            return;
            break;
        case InputFoundParam:
            pc->coordinate = input_value;
            break;
        case InputValue:
            pc->position_command[pc->coordinate] = input_value;
            break;
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
    uint32_t response_timeout = 2000;

    int opt, err;
    char *offset;
    while((opt = getopt(argc, argv, "p:b:a:t:r:")) != -1)
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
            case 'r':
                response_timeout = 1000*atoi(optarg);
                break;
        }
    }

    modbus_t *ctx;

    struct sched_param sched = {
        .sched_priority = 10
    };
    if(sched_setscheduler(0, SCHED_FIFO, &sched) != 0)
    {
        perror("Unable to set scheduler");
        printf("Try:\nsudo setcap \"cap_sys_nice=ep\" %s\n", argv[0]);
        exit(1);
    }

    // Phony baud
    ctx = modbus_new_rtu(devname, 9600, 'N', 8, 1);
    if(!ctx)
    {
        perror("Create modbus context");
        exit(1);
    }

    // Actual baud rate here
    if(configure_modbus_context(ctx, baud, response_timeout))
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
    struct timeval now, dt, last_wakeup;
    suseconds_t sleep;
    gettimeofday(&last_wakeup, NULL);
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
                gettimeofday(&now, NULL);
                timersub(&now, &last_wakeup, &dt);
                sleep = period*1000 - dt.tv_usec;
                if(sleep>0)
                    usleep(sleep);
                gettimeofday(&last_wakeup, NULL);
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
