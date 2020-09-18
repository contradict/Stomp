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
    SAVE,
};

enum InputAction
{
    InputNoAction = 0,
    InputProcessed = -1,
    InputFoundParam = -2,
    InputValue = -3
};

struct InputContext
{
    char input_string[256];
    int input_pos;
    bool input_search;
    int field_count;
    const char **input_fields;
};

struct ErrorContext
{
    char error[256];
    bool iserror;
};

#define NUM_SENSOR_PARAMS 6
struct SensorContext {
    int joint;
    int param;
    int param_max;
    int16_t params[3][NUM_SENSOR_PARAMS];
    bool minmax_mode;
    bool isinit[3];
    int16_t sensor_data[3][6];
    float current_v[3];
    float min_v[3];
    float max_v[3];
    struct InputContext input;
    bool stoponerror;
    struct ErrorContext error[3];
};

#define LENGTH_SCALE 1e2f
const char *sensor_param_names[NUM_SENSOR_PARAMS] = {
       " Vmin",    " Vmax",   " Tmin",     " Tmax",
       " lmin",    " lmax"};
const char *sensor_param_formats[NUM_SENSOR_PARAMS] = {
    "%6.3f", "%6.3f", "%6.1f", "%6.1f", "%6.2f", "%6.2f"};
uint16_t sensor_param_register[NUM_SENSOR_PARAMS] = {
    HSensorVmin, HSensorVmax, HSensorThetamin, HSensorThetamax,
    HCylinderLengthMin, HCylinderLengthMax};
const float sensor_param_scale[NUM_SENSOR_PARAMS] = {
    1000.0f, 1000.0f, 1000.0f / 180.0f * M_PI, 1000.0f / 180.0f * M_PI,
    LENGTH_SCALE, LENGTH_SCALE};
const int16_t sensor_param_min[NUM_SENSOR_PARAMS] = {
    0, 0, -M_PI*1000, -M_PI*1000, 0, 0};
const int16_t sensor_param_max[NUM_SENSOR_PARAMS] = {
    5100, 5100, M_PI*1000, M_PI*1000, 12000, 12000};

#define NUM_SERVO_PARAMS 15

struct ServoContext
{
    bool stoponerror;
    uint16_t params[3][NUM_SERVO_PARAMS];
    int16_t base_pressure[3];
    int16_t rod_pressure[3];
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

const char *servo_param_names[NUM_SERVO_PARAMS + 2] = {
    " P",    " D",    " F",     " O",
    " A",    " C",    "MN",     "MX",
    "RU",    "RD",    "DB",     "DI",    " V",
    " W",    "CM",    "sf",    "sa"};
const char *servo_param_formats[NUM_SERVO_PARAMS + 2] = {
    "%6.1f", "%6.1f", "%6.1f", "%6.1f",
    "%6.4f", "%6.4f", "%6.1f", "%6.1f",
    "%6.1f", "%6.1f", "%6.1f", "%6.1f", "%6.1f",
    "%6.1f", "%6.1f", "%6.2f", "%6.1f"};
uint16_t servo_param_register[NUM_SERVO_PARAMS] = {
    HProportionalGain, HDerivativeGain, HForceDamping, HOffset,
    HAreaRatio, HCylinderBore, HMinimumPosition, HMaximumPosition,
    HRampUp, HRampDown, HDeadBand, HDitherAmplitude, HValveOffset,
    HFeedbackLowpass, HCachedDigitalCommand};
const float servo_param_scale[NUM_SERVO_PARAMS] = {
      10.0f,   10.0f,   10.0f,   10.0f,
      32767.0f, 1024.0f, 10.0f,  10.0f,
      10.0f,   10.0f, 10.0f,   10.0f,   10.0f,
      10.0f, 40.95f};
const int16_t param_max[NUM_SERVO_PARAMS] = {
       1000,   1000,     1000,    1000,
       32767, 32767,     1000,    1000,
       1000,    1000,    1000,    1000,  5000,
       5000,   4095};

const int FB_LP=13;
const int SERVO_MEAS=14;
const int SIGGEN_FREQ=15;
const int SIGGEN_AMPL=16;

struct PositionContext {
    float step;
    int coordinate;
    float position_display[3];
    float position_command[3];
    struct ErrorContext error;
    struct InputContext input;
};

const char *coordinate_names[3] = {"x", "y", "z"};

struct SaveContext {
    int param_max;
    int param;
    uint16_t newaddress;
    struct InputContext input;
};

static int joint[3] = {CURL_BASE, SWING_BASE, LIFT_BASE};
static char *joint_name[3] = {"Curl ", "Swing", "Lift "};

#define CLIP(x, mn, mx) (((x)<(mn))?(mn):(((x)>(mx))?(mx):(x)))

static void input_display(int y, int x, struct InputContext *ic);
static enum InputAction input_handle_key(struct InputContext *ic, int ch, float *v);
static int showerror(struct ErrorContext *ec, const char *msg, int err, struct timeval *dt);
void error_display(int y, int x, struct ErrorContext *ec);

void sensor_init_joint(modbus_t *mctx, struct SensorContext *sc, int j)
{
    uint16_t data[6];
    int err;

    err = modbus_read_registers(mctx, joint[j] + HSensorVmin, 6, data);
    showerror(&sc->error[j], "Read calibration failed", err, NULL);
    if(err == -1)
    {
        sc->isinit[j] = false;
    }
    else
    {
        memcpy(sc->params[j], data, sizeof(data));
        sc->error[j].iserror = false;
        sc->error[j].error[0] = 0;
        sc->isinit[j] = true;
    }
}

void sensor_init(modbus_t *mctx, void *sctx)
{
    struct SensorContext *sc = (struct SensorContext *)sctx;
    sc->param = 0;
    sc->param_max = NUM_SENSOR_PARAMS;
    sc->minmax_mode = false;
    sc->stoponerror = false;
    sc->joint = 0;
    sc->input.input_pos = 0;
    sc->input.input_string[0] = 0;
    sc->input.input_search = false;
    sc->input.input_fields = sensor_param_names;
    sc->input.field_count = NUM_SENSOR_PARAMS;
    for(int j=0;j<3;j++)
    {
        sensor_init_joint(mctx, sc, j);
    }
    clear();
}

void sensor_display(modbus_t *mctx, void *sctx)
{
    int err;
    char display[256];
    uint16_t data[6];
    bool anyerr = false;
    struct SensorContext *sc = (struct SensorContext *)sctx;
    struct timeval before, after;

    for(int j=0;j<3;j++)
    {
        if(!sc->stoponerror | !anyerr)
        {
            gettimeofday(&before, NULL);
            err = modbus_read_input_registers(mctx, joint[j] + ISensorVoltage, 6, data);
            gettimeofday(&after, NULL);
            timersub(&after, &before, &after);
            showerror(&sc->error[j], "Read sensor failed", err, &after);
            if(err != -1)
            {
                sc->current_v[j] = data[0];
                memcpy(sc->sensor_data[j], data, sizeof(data));
                if(sc->minmax_mode)
                {
                    sc->min_v[j] = MIN(sc->min_v[j], sc->current_v[j] / sensor_param_scale[0]);
                    sc->max_v[j] = MAX(sc->max_v[j], sc->current_v[j] / sensor_param_scale[1]);
                }
            }
            else
            {
                anyerr = true;
            }
            if(!anyerr && !sc->isinit[j])
            {
                sensor_init_joint(mctx, sc, j);
            }
        }
        move(3, 10);
        if(sc->minmax_mode)
        {
            printw("minmax");
        }
        else
        {
            clrtoeol();
        }
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
        snprintf(display, sizeof(display), " s: %5.3fV a: %06.1fd f: %06.3fV l: %06.2fcm b: %04x r: %04x",
                 sc->sensor_data[j][0] / 1000.0f, sc->sensor_data[j][1] / 1000.0f * 180.0f / M_PI,
                 sc->sensor_data[j][2] / 1000.0f, sc->sensor_data[j][3] / LENGTH_SCALE,
                 sc->sensor_data[j][4], sc->sensor_data[j][5]);
        addstr(display);
        move(4 + 4 * j + 2, 2);
        clrtoeol();
        for(int p=0;p<6;p++)
        {
            move(4 + 4 * j + 2, 2 + 12 * p);
            if(sc->joint == j && sc->param == p)
            {
                attron(A_BOLD);
            }
            printw("%s:", sensor_param_names[p]);
            if(sc->joint == j && sc->param == p)
            {
                attroff(A_BOLD);
            }
            printw(sensor_param_formats[p], sc->params[j][p] / sensor_param_scale[p]);
        }
        move(4 + 4 * j + 3, 6 + 2);
        clrtoeol();
        if(sc->minmax_mode)
        {
            printw(sensor_param_formats[0], sc->min_v[j]);
            move(4 + 4 * j + 3, 12 + 6 + 2);
            printw(sensor_param_formats[1], sc->max_v[j]);
        }
    }
    move(4 + 4*sc->joint + 2, 2 + 12*sc->param);
}

void set_sensor_param(modbus_t *mctx, struct SensorContext *sc, float value)
{
    int err;
    int16_t pv;
    if(sc->param<NUM_SENSOR_PARAMS)
    {
        pv =  CLIP(value * sensor_param_scale[sc->param], sensor_param_min[sc->param], sensor_param_max[sc->param]);
        err = modbus_write_register(
                mctx, joint[sc->joint] + sensor_param_register[sc->param], pv);
        showerror(&sc->error[sc->joint], "change param failed", err, NULL);
        if(err != -1)
        {
            sc->params[sc->joint][sc->param] = pv;
        }
    }
}

void increment_sensor_param(modbus_t *mctx, struct SensorContext *sc, int increment)
{
    int err;
    int16_t pv;
    if(sc->param<NUM_SENSOR_PARAMS)
    {
        pv = CLIP(sc->params[sc->joint][sc->param] + increment,
                  sensor_param_min[sc->param], sensor_param_max[sc->param]);
        err = modbus_write_register(
                mctx, joint[sc->joint] + sensor_param_register[sc->param], pv);
        showerror(&sc->error[sc->joint], "change param failed", err, NULL);
        if(err != -1)
        {
            sc->params[sc->joint][sc->param] = pv;
        }
    }
}


void sensor_handle_key(modbus_t *mctx, void *sctx, int ch)
{
    struct SensorContext *sc = (struct SensorContext *)sctx;
    float input_value;
    int savejoint, saveparam;
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
            set_sensor_param(mctx, sc, input_value);
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
        case '\x09': // TAB
            sc->joint = (sc->joint + 1) % 3;
            break;
        case KEY_BTAB:
            sc->joint = (sc->joint - 1) % 3;
            if(sc->joint < 0) sc->joint += 3;
            break;
        case 'w':
            increment_sensor_param(mctx, sc, 1);
            break;
        case 'W':
            increment_sensor_param(mctx, sc, 10);
            break;
        case 'a':
            increment_sensor_param(mctx, sc, -1);
            break;
        case 'A':
            increment_sensor_param(mctx, sc, -10);
            break;
        case KEY_F(12):
            for(int j=0;j<JOINT_COUNT;j++)
                sc->error[j].iserror = false;
            break;
        case 'e':
            sc->stoponerror ^= 1;
            break;
        case 'M':
            sc->minmax_mode ^= true;
            if(sc->minmax_mode)
            {
                for(int j=0;j<JOINT_COUNT;j++)
                {
                    sc->min_v[j] = sc->max_v[j] = sc->current_v[j] / sensor_param_scale[0];
                }
            }
            break;
        case 'C':
            if(sc->minmax_mode)
            {
                saveparam = sc->param;
                savejoint = sc->joint;
                for(int j=0;j<3;j++)
                {
                    sc->joint = j;
                    sc->param = 0;
                    set_sensor_param(mctx, sc, sc->min_v[j]);
                    sc->param = 1;
                    set_sensor_param(mctx, sc, sc->max_v[j]);
                }
                sc->param = saveparam;
                sc->joint = savejoint;
            }
            break;
    }
}

void error_display(int y, int x, struct ErrorContext *ec)
{
    if(ec->iserror)
    {
        move(y, x);
        addstr(ec->error);
    }
}

void input_display(int y, int x, struct InputContext *ic)
{
    if(ic->input_pos>0)
    {
        move(y, x);
        addstr(ic->input_string);
    }
}

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

int showerror(struct ErrorContext *ec, const char *msg, int err, struct timeval *dt)
{
    int modbus_err = errno;
    if(err == -1 && !ec->iserror)
    {
        if(dt)
        {
            snprintf(ec->error, 256, "%s(%ld): %s", msg, dt->tv_usec, modbus_strerror(modbus_err));
        }
        else
        {
            snprintf(ec->error, 256, "%s: %s", msg, modbus_strerror(modbus_err));
        }
        ec->iserror = true;
    }
    return modbus_err;
}

void servo_init(modbus_t *mctx, void *sctx)
{
    struct ServoContext *sc = (struct ServoContext *)sctx;
    uint16_t data[1];
    sc->stoponerror = true;
    sc->joint = 0;
    sc->param = 0;
    sc->param_max = NUM_SERVO_PARAMS;
    sc->siggen_enable = false;
    sc->input.input_pos = 0;
    sc->input.input_string[0] = 0;
    sc->input.input_search = false;
    sc->input.input_fields = servo_param_names;
    sc->input.field_count = NUM_SERVO_PARAMS;
    //int n;
    int err;
    uint32_t sto_sec, sto_usec;
    modbus_get_response_timeout(mctx, &sto_sec, &sto_usec);
    modbus_set_response_timeout(mctx, 0, 100000);
    for(int j=0;j<3;j++)
    {
        sc->error[j].iserror = false;
        sc->error[j].error[0] = 0;
        sc->isinit[j] = true;
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
            sc->params[j][SERVO_MEAS] = data[0];
        }
        err = modbus_read_registers(
                mctx, joint[j]+HFeedbackLowpass, 1, data);
        showerror(&sc->error[j], "Initial Lowpass Read failed", err, NULL);
        if(err != -1)
        {
            sc->params[j][FB_LP] = data[0];
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

int step_signal_generator(modbus_t *mctx, struct ServoContext *sc)
{
    struct timeval now, dt;
    int err = 0;
    if(sc->siggen_enable)
    {
        gettimeofday(&now, NULL);
        timersub(&now, &sc->last_siggen_output, &dt);
        sc->siggen_phase += 2.0f*M_PI*sc->siggen_frequency * dt.tv_usec / 1.0e6f;
        sc->siggen_signal = 2047.0f + 40.95f * sc->siggen_amplitude * sinf(sc->siggen_phase);
        while(sc->siggen_phase > 2.0f * M_PI) sc->siggen_phase -= 2.0f * M_PI;
        if(sc->siggen_start || (fabs(sc->measured[sc->joint] - sc->siggen_signal) < 100))
        {
            sc->params[sc->joint][SERVO_MEAS] = sc->siggen_signal;
            err = modbus_write_register(
                mctx, joint[sc->joint] + HDigitalCommand,
                sc->params[sc->joint][SERVO_MEAS]);
            showerror(&sc->error[sc->joint], "siggen set failed", err, NULL);
            sc->siggen_start = true;
        }
        memcpy(&sc->last_siggen_output, &now, sizeof(struct timeval));
    }
    return err;
}

void servo_display(modbus_t *mctx, void *sctx)
{
    const int SERVO_ROWS=7;
    const int SERVO_COL_SPACE=12;
    struct ServoContext *sc = (struct ServoContext *)sctx;
    uint16_t data[3];
    int err;
    struct timeval before, after;

    bool anyerr = false;
    for(int j=0;j<3;j++)
    {
        anyerr |= sc->error[j].iserror;
    }

    for(int j=0;j<3;j++)
    {
        if(!sc->stoponerror | !anyerr)
        {
            gettimeofday(&before, NULL);
            err = modbus_read_input_registers(mctx, joint[j]+ICachedBaseEndPressure, 3, data);
            gettimeofday(&after, NULL);
            timersub(&after, &before, &after);
            showerror(&sc->error[j], "Read feedback failed", err, &after);
            if(err != -1)
            {
                sc->base_pressure[j] = data[0];
                sc->rod_pressure[j] = data[1];
                sc->measured[j] = data[2];
            }
            else
            {
                anyerr = true;
            }
        }
        if(!sc->stoponerror | !anyerr)
        {
            if(sc->joint == j)
            {
                anyerr = (-1 == step_signal_generator(mctx, sc));
            }
        }
        move(4 + SERVO_ROWS * j + 0, 2);
        clrtoeol();
        if(sc->joint == j)
            attron(A_BOLD);
        printw("%s(%c)", joint_name[j], sc->isinit[j]?'t':'f');
        attroff(A_BOLD);
        if(j == sc->joint)
            input_display(4 + SERVO_ROWS * j + 0, 2 + 9, &sc->input);
        error_display(4 + SERVO_ROWS * j + 0, 2 + 15, &sc->error[j]);
        move(4 + SERVO_ROWS * j + 1, 2);
        clrtoeol();
        for(int p=0;p<4;p++)
        {
            move(4 + SERVO_ROWS * j + 1, 2 + SERVO_COL_SPACE * p);
            if(sc->joint == j && sc->param == p)
            {
                attron(A_BOLD);
            }
            printw("%s:", servo_param_names[p]);
            if(sc->joint == j && sc->param == p)
            {
                attroff(A_BOLD);
            }
            printw(servo_param_formats[p], sc->params[j][p] / servo_param_scale[p]);
        }
        move(4 + SERVO_ROWS * j + 2, 2);
        clrtoeol();
        for(int p=4;p<8;p++)
        {
            move(4 + SERVO_ROWS * j + 2, 2 + SERVO_COL_SPACE * (p - 4));
            if(sc->joint == j && sc->param == p)
            {
                attron(A_BOLD);
            }
            printw("%s:", servo_param_names[p]);
            if(sc->joint == j && sc->param == p)
            {
                attroff(A_BOLD);
            }
            printw(servo_param_formats[p], sc->params[j][p] / servo_param_scale[p]);
        }
        move(4 + SERVO_ROWS * j + 3, 2);
        clrtoeol();
        for(int p=8;p<13;p++)
        {
            move(4 + SERVO_ROWS * j + 3, 2 + SERVO_COL_SPACE * (p - 8));
            if(sc->joint == j && sc->param == p)
            {
                attron(A_BOLD);
            }
            printw("%s:", servo_param_names[p]);
            if(sc->joint == j && sc->param == p)
            {
                attroff(A_BOLD);
            }
            printw(servo_param_formats[p], sc->params[j][p] / servo_param_scale[p]);
        }
        move(4 + SERVO_ROWS * j + 4, 2);
        clrtoeol();
        for(int p=13;p<NUM_SERVO_PARAMS;p++)
        {
            move(4 + SERVO_ROWS * j + 4, 2 + SERVO_COL_SPACE * (p - 13));
            if(sc->joint == j && sc->param == p)
            {
                attron(A_BOLD);
            }
            printw("%s:", servo_param_names[p]);
            if(sc->joint == j && sc->param == p)
            {
                attroff(A_BOLD);
            }
            printw(servo_param_formats[p], sc->params[j][p] / servo_param_scale[p]);
        }
        if(sc->siggen_enable && j == sc->joint)
        {
            for(int p=NUM_SERVO_PARAMS;p<NUM_SERVO_PARAMS + 2; p++)
            {
                move(4 + SERVO_ROWS * j + 4, 2 + SERVO_COL_SPACE * (2 + p - NUM_SERVO_PARAMS));
                if(sc->param == p)
                {
                    attron(A_BOLD);
                }
                printw("%s:", servo_param_names[p]);
                attroff(A_BOLD);
                switch(p-NUM_SERVO_PARAMS)
                {
                    case 0:
                        printw(servo_param_formats[p], sc->siggen_frequency);
                        break;
                    case 1:
                        printw(servo_param_formats[p], sc->siggen_amplitude);
                        break;
                }
            }
            move(4 + SERVO_ROWS * j + 4, 2 + SERVO_COL_SPACE * 4);
            printw("ph:%6.1f", sc->siggen_phase);
            move(4 + SERVO_ROWS * j + 4, 2 + SERVO_COL_SPACE * 5);
            printw("ss:%6.1f", sc->siggen_signal / 40.95);
            
        }
        move(4 + SERVO_ROWS * j + 5, 2 + 1*SERVO_COL_SPACE);
        clrtoeol();
        printw(" m:%6.1f b:%6.1f r:%6.1f",
                sc->measured[j] / 40.95f, sc->base_pressure[j]/25.8f, sc->rod_pressure[j]/25.8f);
    }
    if(sc->param < 4)
        move(4 + SERVO_ROWS*sc->joint + 1, 2 + SERVO_COL_SPACE*sc->param);
    else if(sc->param < 8)
        move(4 + SERVO_ROWS*sc->joint + 2, 2 + SERVO_COL_SPACE*(sc->param - 4));
    else if(sc->param < 13)
        move(4 + SERVO_ROWS*sc->joint + 3, 2 + SERVO_COL_SPACE*(sc->param - 8));
    else
        move(4 + SERVO_ROWS*sc->joint + 4, 2 + SERVO_COL_SPACE*(sc->param - 13));
}

void set_param(modbus_t *mctx, struct ServoContext *sc, float value)
{
    int err;
    int16_t pv;
    if(sc->param<NUM_SERVO_PARAMS)
    {
        pv =  CLIP(value * servo_param_scale[sc->param], 0, param_max[sc->param]);
        err = modbus_write_register(
                mctx, joint[sc->joint] + servo_param_register[sc->param], pv);
        showerror(&sc->error[sc->joint], "change param failed", err, NULL);
        if(err != -1)
        {
            sc->params[sc->joint][sc->param] = pv;
        }
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
    int16_t pv;
    if(sc->param<NUM_SERVO_PARAMS)
    {
        pv = CLIP(sc->params[sc->joint][sc->param] + increment,
                  0, param_max[sc->param]);
        err = modbus_write_register(
                mctx, joint[sc->joint] + servo_param_register[sc->param], pv);
        showerror(&sc->error[sc->joint], "change param failed", err, NULL);
        if(err != -1)
        {
            sc->params[sc->joint][sc->param] = pv;
        }
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
            sc->siggen_enable = false;
            sc->siggen_start = false;
            break;
        case KEY_BTAB:
            sc->joint = (sc->joint - 1) % 3;
            if(sc->joint < 0) sc->joint += 3;
            sc->siggen_enable = false;
            sc->siggen_start = false;
            break;
        case 'C':
            if(sc->joint != 0)
            {
                sc->siggen_enable = false;
                sc->siggen_start = false;
            }
            sc->joint = 0;
            break;
        case 'S':
            if(sc->joint != 1)
            {
                sc->siggen_enable = false;
                sc->siggen_start = false;
            }
            sc->joint = 1;
            break;
        case 'L':
            if(sc->joint != 2)
            {
                sc->siggen_enable = false;
                sc->siggen_start = false;
            }
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
            for(int j=0;j<JOINT_COUNT;j++)
                sc->error[j].iserror = false;
            break;
        case 'e':
            sc->stoponerror ^= 1;
            break;
    }
}

void position_init(modbus_t *mctx, void *pctx)
{
    struct PositionContext *pc = (struct PositionContext *)pctx;
    uint16_t pos[3];
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
    err = modbus_read_registers(mctx, ToeXPosition, 3, pos);
    showerror(&pc->error, "Read initial position failed", err, NULL);
    if(err != -1)
    {
        for(int i=0;i<3;i++)
        {
            pc->position_display[i] = ((int16_t *)pos)[i] / LENGTH_SCALE;
            pc->position_command[i] = ((int16_t *)pos)[i] / LENGTH_SCALE;
        }
    }
    clear();
}

void position_display(modbus_t *mctx, void *pctx)
{
    struct PositionContext *pc = (struct PositionContext *)pctx;
    int err;
    uint16_t pos[3];

    err = modbus_read_registers(mctx, ToeXPosition, 3, pos);
    showerror(&pc->error, "Read position failed", err, NULL);
    if(err != -1)
    {
        for(int i=0;i<3;i++)
            pc->position_display[i] = ((int16_t *)pos)[i] / LENGTH_SCALE;
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
    uint16_t pos[3];
    int err;
    for(int c=0;c<3;c++)
        ((int16_t *)pos)[c] = LENGTH_SCALE * pc->position_command[c];
    err = modbus_write_registers(mctx, ToeXPosition, 3, pos);
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

#define NUM_SAVE_PARAMS 1
const char *save_param_names[NUM_SAVE_PARAMS] = {"Addr"};
const char *save_param_formats[NUM_SAVE_PARAMS] = {"0x%02x"};
uint16_t save_param_register[NUM_SAVE_PARAMS] = {HMODBUSAddress};
const float save_param_scale[NUM_SAVE_PARAMS] = {1.0f};
const int16_t save_param_max[NUM_SERVO_PARAMS] = {255};
const int16_t save_param_min[NUM_SERVO_PARAMS] = {0};

void save_init(modbus_t *mctx, void *sctx)
{
    (void)mctx;
    struct SaveContext *sc = (struct SaveContext *)sctx;
    sc->param_max = 1;
    sc->param = 0;
    sc->newaddress = 0x00;
    sc->input.input_pos = 0;
    sc->input.input_string[0] = 0;
    sc->input.input_search = false;
    sc->input.input_fields = save_param_names;
    sc->input.field_count = 1;
    clear();
 }

void save_display(modbus_t *mctx, void *sctx)
{
    struct SaveContext *sc = (struct SaveContext *)sctx;
    //int err;
    uint8_t save;
    uint16_t address;

    modbus_read_bits(mctx, CSaveConstants, 1, &save);
    move(4, 2);
    clrtoeol();
    printw("Saved: %s", save ? "t":"f");
    input_display(4, 12, &sc->input);

    modbus_read_registers(mctx, HMODBUSAddress, 1, &address);
    move(5, 2);
    clrtoeol();
    printw("Address: 0x%02x", address);

    move(5, 17);
    printw("New Address: 0x%02x", sc->newaddress);
}

void save_handle_key(modbus_t *mctx, void *sctx, int ch)
{
    struct SaveContext *sc = (struct SaveContext *)sctx;
    float input_value;
    //int err;
    enum InputAction input;
    input = input_handle_key(&sc->input, ch, &input_value);
    uint8_t saveval;
    switch(input)
    {
        case InputNoAction:
            break;
        case InputProcessed:
            return;
            break;
        case InputValue:
            sc->newaddress = input_value;
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
        case 'V':
            saveval = 0x01;
            modbus_write_bits(mctx, CSaveConstants, 1, &saveval);
            break;
        case 'A':
            modbus_write_registers(mctx, HMODBUSAddress, 1, &(sc->newaddress));
            modbus_set_slave(mctx, sc->newaddress);
            break;
    }
}

int main(int argc, char **argv)
{
    char *devname = "/dev/ttyS4";
    uint32_t baud = 1000000;
    int addr = 0x55;
    int period = 100;
    uint32_t response_timeout = 2000;

    int opt;
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
                    sscanf(offset + 1, "%x", &addr);
                }
                else
                {
                    addr = atoi(optarg);
                }
                printf("address: 0x%x\n", addr);
                break;
            case 't':
                period = atoi(optarg);
                break;
            case 'r':
                response_timeout = 1000*atoi(optarg);
                break;
        }
    }

    uint8_t address = 0xFF & addr;

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

    if(create_modbus_interface(devname, baud, 50, response_timeout, &ctx))
    {
        exit(1);
    }

    modbus_set_slave(ctx, address);

    struct ServoContext servo;
    struct SensorContext sensor;
    struct PositionContext position;
    struct SaveContext save;

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
    sensor_init(ctx, &sensor);
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
                    case SAVE:
                        save_display(ctx, &save);
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
            case KEY_F(4):
                save_init(ctx, &save);
                mode = SAVE;
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
                    case SAVE:
                        save_init(ctx, &save);
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
                    case SAVE:
                        save_handle_key(ctx, &save, ch);
                        break;
                }
        }
    }

    endwin();

    return 0;
}
