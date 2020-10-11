#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>
#include <stdatomic.h>
#include <pthread.h>

#include "joint.h"
#include "modbus_register_map.h"
#include "sclog4c/sclog4c.h"

#include "modbus_device.h"
#include "leg_control/leg_thread.h"
#include "leg_control/toml_utils.h"
#include "leg_control/rate_timer.h"
#include "leg_control/modbus_utils.h"
#include "lcm/stomp_telemetry_leg.h"
#include "lcm/stomp_modbus.h"

#define CLIP(x, mn, mx) (((x)<(mn))?(mn):(((x)>(mx))?(mx):(x)))

const float deg2rad = M_PI / 180.0f;

enum legs_control_mode {
    mode_all_init,
    mode_all_ping,
    mode_all_pos_disable,
    mode_all_pos_enable,
    mode_all_air_on,
    mode_all_gain_zero,
    mode_all_gain_operational_free,
    mode_all_gain_operational_walk,
    mode_all_air_vent_lock,
    mode_all_air_vent_free,
    mode_all_walk,
};

enum leg_control_mode {
    mode_move_start,
    mode_walk,

    mode_set_gain_operational,
    mode_gain_operational,

    mode_set_position,
    mode_error_zero,

    mode_set_gain_zero,
    mode_gain_zero,

    mode_ping,
    mode_ready,
};

struct leg_thread_state {
    struct leg_thread_definition *definition;
    pthread_t thread;
    modbus_t *ctx;
    ringbuf_worker_t *telemetry_worker;
    ringbuf_worker_t *response_worker;
    int nlegs;
    struct leg_description* legs;
    struct joint_gains *joint_gains;
    int nsteps;
    struct step *steps;
    int ngaits;
    char **gait_selections;
    struct gait *gaits;
    int current_gait;
    float toe_position_tolerance;
    float telemetry_frequency;
    float telemetry_period_smoothing;
    float forward_deadband;
    float angular_deadband;
    float support_pressure;
    float desired_ride_height;
    float ride_height_igain;
    float ride_height_pgain;
    float ride_height_integrator_limit;
    float ride_height_scale;
    atomic_bool shouldrun;
    struct rate_timer *timer;
    enum leg_control_mode *leg_mode;
    enum legs_control_mode legs_mode;
    bool valid_measurements;
    float (*toe_position_measured)[3];
    float (*base_end_pressure)[3];
    float (*rod_end_pressure)[3];
    float (*commanded_toe_positions)[3];
    float (*initial_toe_positions)[3];
    float *ride_height_integrator;
    float (*leg_scale)[3];
    float (*leg_offset)[3];
    float theta[2];
    float turning_width;
    float min_angle_change_velocity;
    float walk_phase;
    float observed_period;
};

static int set_gain(modbus_t *ctx, uint8_t address, struct joint_gains *gain)
{
    uint32_t sec, saved_timeout;
    modbus_get_response_timeout(ctx, &sec, &saved_timeout);
    modbus_set_response_timeout(ctx, 0, 100000);
    int err = set_servo_gains(ctx, address, &gain->proportional_gain, &gain->derivative_gain, &gain->force_damping, &gain->feedback_lowpass);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Failed to set servo gain for leg address 0x%02x).", address);
    }
    modbus_set_response_timeout(ctx, 0, saved_timeout);
    return err;
}

static int zero_gain(modbus_t *ctx, uint8_t address)
{
    struct joint_gains zerogain;
    bzero(&zerogain, sizeof(zerogain));
    return set_gain(ctx, address, &zerogain);
}

static int find_interpolation_index(const float *nodes, size_t length, float x)
{
    size_t i;
    for(i=0; i<length; i++)
    {
        if((nodes[i] <= x) && (nodes[i+1] > x))
            break;
    }
    if(i==length)
        return -1;
    return i;
}

static void interpolate_value(const float *nodes, const float *values, ssize_t length, int index, float x, float *y)
{
    int next_node = index+1;
    int next_value = next_node==length ? 0 : next_node;
    *y = (x - nodes[index]) * (values[next_value] - values[index]) / (nodes[next_node] - nodes[index]) + values[index];
}

static int interpolate_step(struct step* step, struct gait* gait, int leg_index, float phase, float (*toe_position)[3])
{
    float discard;
    float leg_phase = modff(phase + gait->phase_offsets[leg_index], &discard);
    int index = find_interpolation_index(step->phase, step->npoints, leg_phase);
    if(index<0)
        return index;
    interpolate_value(step->phase, step->X, step->npoints, index, leg_phase, &((*toe_position)[0]));
    interpolate_value(step->phase, step->Y, step->npoints, index, leg_phase, &((*toe_position)[1]));
    interpolate_value(step->phase, step->Z, step->npoints, index, leg_phase, &((*toe_position)[2]));
    return 0;
}

static float deadband(float f, float d)
{
    if(fabsf(f) < d)
        return 0;
    else
        return f - copysignf(d, f);
}

static void compute_walk_velocity(struct leg_thread_state* state, struct leg_control_parameters *p, float (*velocity)[2], float (*theta)[2])
{
    float forward = deadband(p->forward_velocity, state->forward_deadband);
    float angular = deadband(p->angular_velocity, state->angular_deadband);
    float lateral = deadband(p->right_velocity, state->forward_deadband);
    float linear = hypotf(forward, lateral);

    switch(p->motion)
    {
        case MOTION_MODE_STEERING:
            (*velocity)[0] = forward - state->turning_width * angular;
            (*velocity)[1] = forward + state->turning_width * angular;
            (*theta)[0] = (*theta)[1] = 0.0f;
            break;
        case MOTION_MODE_TRANSLATING:
            (*velocity)[1] = (*velocity)[0] = linear;
            (*theta)[0] = (*theta)[1] = atan2f(lateral, forward);
            break;
        case MOTION_MODE_DRIVING:
            (*velocity)[1] = hypotf(forward + state->turning_width * angular, lateral);
            (*theta)[1] = atan2f(lateral, forward + state->turning_width * angular);
            (*velocity)[0] = hypotf(forward - state->turning_width * angular, lateral);
            (*theta)[0] = atan2f(lateral, forward - state->turning_width * angular);
            break;
    }
    if((*velocity)[0] < state->min_angle_change_velocity && (*velocity)[1] < state->min_angle_change_velocity)
    {
        (*theta)[0] = state->theta[0];
        (*theta)[1] = state->theta[1];
    }
    state->theta[0] = (*theta)[0];
    state->theta[1] = (*theta)[1];
}

static float compute_walk_frequency(float (*step_length)[2], float (*velocity)[2])
{
    return MAX(fabsf((*velocity)[0]) / (*step_length)[0], fabsf((*velocity)[1]) / (*step_length)[1]);
}

// pt(s) = [xc, 0] + [sin(t), cos(t)] * s
// || pt || = r_inner
//        or
// || pt || = r_outer
//        or
// arg(pt) = theta_max
// (xc - s sin)^2 + s^2 cos^2 = r^2
// xc^2 - 2 s xc sin + s^2 = r^2
// s^2 - 2 xc sin s + xc^2 - r^2 = 0
// s = (2 xc sin +/- sqrt(4 xc^2 sin^2 - 4 (xc^2 - r^2))) / 2
// s = (xc sin +/- sqrt(xc^2 (sin^2 - 1) + r^2))
// s = (xc sin +/- sqrt(r^2 - xc^2 cos^2))
//
// arctan(s cos / (xc + s sin)) = theta_max
// s cos / (xc + s sin) = tan(theta_max)
// s cos = xc tan(theta_max) + s sin tan(theta_max)
// s cos - s sin tan(theta_max) = xc tan(theta_max)
// s (cos - sin tan(theta_max)) = xc tan(theta_max)
// s = xc tan(theta_max) / (cos - sin tan(theta_max))

static float intersect_working_volume(struct step* step, float theta, float *pxc)
{
    float r_inner = step->r_inner;
    float r_outer = step->r_outer;
    float xc = step->minimum[0];
    if(pxc != 0)
        *pxc = xc;
    float st = sinf(theta), ct=cosf(theta);
    float sgnst = copysignf(1.0f, st);
    float s_rin = sgnst * xc * st - sqrtf(r_inner*r_inner - xc*xc*ct*ct);
    float s_rout = -(sgnst * xc * st - sqrtf(r_outer*r_outer - xc*xc*ct*ct));
    float s_t = fabsf(xc * tanf(step->swing_angle_max) / (ct - st * tanf(step->swing_angle_max)));
    float s_min;
    if(isnan(s_rin))
        s_min = MIN(s_rin, s_rout);
    else
        s_min = s_rout;
    s_min = MIN(s_min, s_t);
    return s_min;
}

static float compute_step_length(struct step* step, struct leg_control_parameters* p, float theta, float *xc)
{
    float l;
    switch(p->motion)
    {
        case MOTION_MODE_STEERING:
            if(xc != 0)
                *xc = step->minimum[0];
            l = step->maximum[1] - step->minimum[1];
            break;
        case MOTION_MODE_TRANSLATING:
        case MOTION_MODE_DRIVING:
            l = 2 * intersect_working_volume(step, theta, xc);
            break;
    }
    return l;
}

static bool anyclose(float *x, int n, float y, float dy)
{
    bool close = false;
    for(int i=0;i<n;i++)
        close |= (fabsf(x[i] - y) < dy);
    return close;
}

static int compute_walk_scale(struct step* step, struct gait* gait, float phase, float (*velocity)[2], float (*length)[2], int nlegs, float (*leg_scale)[3], float (*leg_offset)[3])
{
    // scale slower leg to achieve commanded velocity
    float left_scale, right_scale, scale;
    if(fabsf((*velocity)[1]) <= 1e-4 && fabsf((*velocity)[0]) <= 1e-4)
    {
        left_scale = copysignf(1.0f, (*velocity)[0]);
        right_scale = copysignf(1.0f, (*velocity)[1]);
    }
    else if(fabsf((*velocity)[1]) <= 1e-4 && fabsf((*velocity)[0]) > 1e-4)
    {
        right_scale = 0.0f;
        left_scale = copysignf(1.0f, (*velocity)[0]);
    }
    else
    {
        scale = fabs((*velocity)[0] / (*velocity)[1]);
        if(scale < 1.0)
        {
            left_scale = copysignf(scale, (*velocity)[0]);
            right_scale = copysignf(1.0f, (*velocity)[1]);
        }
        else
        {
            left_scale = copysignf(1.0f, (*velocity)[0]);
            right_scale = copysignf(1.0f/scale, (*velocity)[1]);
        }
    }
    // don't change direction until at the center to avoid lare swings
    for(int l=0;l<nlegs / 2;l++)
    {
        float discard, leg_phase = modff(phase + gait->phase_offsets[l], &discard);
        if((signbit(leg_scale[l][1]) == signbit(right_scale)) ||
                anyclose(step->swap_phase, step->nswap, leg_phase, step->swap_tolerance))
        {
            leg_scale[l][1] = right_scale;
        }
    }
    for(int l=nlegs/2; l<nlegs; l++)
    {
        float discard, leg_phase = modff(phase + gait->phase_offsets[l], &discard);
        if((signbit(leg_scale[l][1]) == signbit(left_scale)) ||
                anyclose(step->swap_phase, step->nswap, leg_phase, step->swap_tolerance))
        {
            leg_scale[l][1] = left_scale;
        }
    }
    // scale to real units with correct length for current step angle
    for(int l=0;l<nlegs/2;l++)
    {
        leg_scale[l][0] = (step->maximum[0] - step->minimum[0]) / 2.0f;
        leg_offset[l][0] = leg_scale[l][0] + step->minimum[0];
        leg_scale[l][2] = (step->maximum[2] - step->minimum[2]) / 2.0f;
        leg_offset[l][2] = leg_scale[l][2] + step->minimum[2];

        leg_scale[l][1] *= (*length)[1] / 2.0f;
        leg_offset[l][1] = 0;
    }
    for(int l=nlegs/2;l<nlegs;l++)
    {
        leg_scale[l][0] = (step->maximum[0] - step->minimum[0]) / 2.0f;
        leg_offset[l][0] = leg_scale[l][0] + step->minimum[0];
        leg_scale[l][2] = (step->maximum[2] - step->minimum[2]) / 2.0f;
        leg_offset[l][2] = leg_scale[l][2] + step->minimum[2];

        leg_scale[l][1] *= (*length)[0] / 2.0f;
        leg_offset[l][1] = 0;
    }
    return 0;
}

static void rotate_leg_path(float theta, float xc, float (*pos)[3])
{
    float st=sinf(theta), ct=cosf(theta);
    float x = (*pos)[0];
    (*pos)[0] =  (x - xc)*ct + (*pos)[1]*st + xc;
    (*pos)[1] = -(x - xc)*st + (*pos)[1]*ct;
}

static bool servo_ride_height(struct leg_thread_state* st, float extra, float *height_offset, float igain, float pgain)
{
    float target = st->desired_ride_height + st->ride_height_scale * extra;
    float mean_error = 0;
    if(st->valid_measurements)
    {
        bool down[st->nlegs];
        int ndown=0;
        for(int l=0; l<st->nlegs; l++)
        {
            float dP = (st->base_end_pressure[l][JOINT_LIFT] - st->rod_end_pressure[l][JOINT_LIFT]);
            down[l] = dP > st->support_pressure;
            if(down[l] && st->leg_mode[l] == mode_walk)
            {
                float error = target - st->toe_position_measured[l][2];
                st->ride_height_integrator[l] += error;
                st->ride_height_integrator[l] = CLIP(st->ride_height_integrator[l],
                                                     -st->ride_height_integrator_limit,
                                                      st->ride_height_integrator_limit);
                mean_error += error;
                ndown++;
            }
        }
        if(ndown > 0)
            mean_error /= ndown;
        if(sclog4c_level <= SL4C_FINE)
        {
            char message[256];
            int nchar;
            nchar = snprintf(message, sizeof(message), "down: [");
            for(int l=0; l<st->nlegs; l++)
            {
                nchar += snprintf(message + nchar, sizeof(message) - nchar, "%s%s",
                        down[l] ? "d" : "u", l<st->nlegs-1 ? "," : "]");
            }
            logm(SL4C_FINE, "%s", message);
        }
    }
    for(int l=0; l<st->nlegs; l++)
    {
        height_offset[l] = st->ride_height_scale * extra + st->ride_height_integrator[l] * igain + mean_error * pgain;
    }
    if(sclog4c_level <= SL4C_FINE)
    {
        char message[256];
        int nchar;
        nchar = snprintf(message, sizeof(message), "i: %6.3f p: %6.3f i: [", igain, pgain);
        for(int l=0; l<st->nlegs; l++)
        {
            nchar += snprintf(message + nchar, sizeof(message) - nchar, "%5.3f%s",
                    st->ride_height_integrator[l], l<st->nlegs-1 ? ", " : "]");
        }
        logm(SL4C_FINE, "%s", message);
    }
    return true;
}

static int compute_toe_positions(struct leg_thread_state* state, struct leg_control_parameters *p, float dt)
{
    struct gait *gait = (state->gaits+state->current_gait);
    struct step *step = (state->steps+gait->step_index);
    float velocity[2];
    float theta[2], xc;
    float step_length[2];
    compute_walk_velocity(state, p, &velocity, &theta);
    step_length[0] = compute_step_length(step, p, theta[0], &xc);
    step_length[1] = compute_step_length(step, p, theta[1], NULL);
    float frequency = compute_walk_frequency(&step_length, &velocity);
    float dummy;
    state->walk_phase = MAX(modff(state->walk_phase + frequency * dt, &dummy), 0.0f);
    compute_walk_scale(step, gait, state->walk_phase, &velocity, &step_length, state->nlegs, state->leg_scale, state->leg_offset);

    float height_offset[state->nlegs];
    bool have_offset = servo_ride_height(state, p->ride_height, height_offset, state->ride_height_igain, state->ride_height_pgain);

    int ret = 0;
    for(int leg=0; leg<state->nlegs; leg++)
    {
        interpolate_step(step, gait, leg, state->walk_phase, &state->commanded_toe_positions[leg]);
        // Orient step for each leg, only handles 180 rotation about Z to get
        // left/right side correct
        state->commanded_toe_positions[leg][1] *= copysignf(1.0f, cosf(state->legs[leg].orientation[2] * deg2rad));
        // apply scale
        for(int i=0;i<3;i++)
            state->commanded_toe_positions[leg][i] = state->commanded_toe_positions[leg][i] * state->leg_scale[leg][i] + state->leg_offset[leg][i];
        rotate_leg_path(leg<state->nlegs/2 ? theta[1] : theta[0], xc, &state->commanded_toe_positions[leg]);
        if(have_offset)
            state->commanded_toe_positions[leg][2] += height_offset[leg];
    }
    return ret;
}

static int move_towards_gait(modbus_t* ctx, uint8_t address, float (*measured_toe_position)[3], float (*gait_toe_position)[3], float tolerance)
{
    float distance = 0;
    int ret = 0, err;
    for(int i=0; i<3; i++)
        distance += pow(((*measured_toe_position)[i] - (*gait_toe_position)[i]), 2);
    distance = sqrtf(distance);
    logm(SL4C_FINE, "addr 0x%02x m:[%5.3f, %5.3f, %5.3f] g:[%5.3f, %5.3f, %5.3f]",
         address,
         (*measured_toe_position)[0], (*measured_toe_position)[1], (*measured_toe_position)[2],
         (*gait_toe_position)[0], (*gait_toe_position)[1], (*gait_toe_position)[2]);
    if(distance<tolerance)
    {
        err = set_toe_postion(ctx, address, gait_toe_position);
        if(err == -1)
            ret = -1;
        else
            ret = 1;
        logm(SL4C_DEBUG, "addr 0x%02x dist %5.3f", address, distance);
    }
    else
    {
        float interp = tolerance / distance;
        float interpolated_toe_position[3];
        for(int i=0; i<3; i++)
            interpolated_toe_position[i] = (*measured_toe_position)[i] * (1.0f - interp) + (*gait_toe_position)[i] * interp;
        err = set_toe_postion(ctx, address, &interpolated_toe_position);
        if(err == -1)
            ret = -1;
        logm(SL4C_DEBUG, "addr 0x%02x dist %5.3f interp %5.3f", address, distance, interp);
        logm(SL4C_FINE, "addr 0x%02x i:[%5.3f, %5.3f, %5.3f]", address,
             interpolated_toe_position[0], interpolated_toe_position[1], interpolated_toe_position[2]);
    }
    return ret;
}

static int read_parameters(struct queue *pq, struct leg_control_parameters *p)
{
    size_t offset, sp;
    sp = sizeof(struct leg_control_parameters);
    size_t s = ringbuf_consume(pq->ringbuf, &offset);
    if(s >= sp)
    {
        memcpy(p, pq->buffer + offset + s - sp, sp);
        ringbuf_release(pq->ringbuf, s);
    }
    return s;
}

static void air_vent()
{
}

static void air_on()
{
}

enum leg_control_mode run_leg_state_machine(enum leg_control_mode mode,
                                            uint8_t address,
                                            struct joint_gains *gains,
                                            float (*measured_toe_position)[3],
                                            bool valid_measurements,
                                            float (*gait_toe_position)[3],
                                            float gait_distance_tolerance,
                                            modbus_t *ctx)
{
    enum leg_control_mode newmode = mode;
    int err;
    switch(mode)
    {
        case mode_move_start:
            // move legs to start of cycle
            if(valid_measurements)
            {
                err = move_towards_gait(ctx, address, measured_toe_position, gait_toe_position, gait_distance_tolerance);
                if(err == -1)
                {
                    //newmode = mode_air_vent_init;
                    logm(SL4C_ERROR, "Position ramp error: %s.",
                         modbus_strerror(errno));
                }
                else if(err == 1)
                {
                    newmode = mode_walk;
                    logm(SL4C_INFO, "move_start->walk");
                }
            }
            break;
        case mode_walk:
            err = set_toe_postion(ctx, address, gait_toe_position);
            if(err == -1)
            {
                newmode = mode_move_start;
                logm(SL4C_ERROR, "walk failed.");
                logm(SL4C_INFO, "walk->move_start.");
            }
            break;
        case mode_set_gain_operational:
            err = set_gain(ctx, address, gains);
            if(err == 0)
            {
                newmode = mode_gain_operational;
                logm(SL4C_INFO, "Leg address 0x%02x set_gain->gain_operational.", address);
            }
            else
            {
                logm(SL4C_ERROR, "Leg address 0x%02x set operational gain failed.", address);
            }
            break;
        case mode_gain_operational:
            break;
        case mode_set_position:
            if(valid_measurements)
            {
                err = set_toe_postion(ctx, address, measured_toe_position);
                if(err == -1)
                {
                    logm(SL4C_ERROR, "Leg address 0x%02x set position failed: %s.",
                         address, modbus_strerror(errno));
                }
                else
                {
                    newmode = mode_error_zero;
                    logm(SL4C_INFO, "Leg address 0x%02x setpos->errorzero.", address);
                }
            }
            break;
        case mode_error_zero:
            break;
        case mode_set_gain_zero:
            err = zero_gain(ctx, address);
            if(err == 0)
            {
                newmode = mode_gain_zero;
                logm(SL4C_INFO, "Leg address 0x%02x set_gain->gain_zero.", address);
            }
            else
            {
                logm(SL4C_ERROR, "Leg address 0x%02x zero gain failed.", address);
            }
            break;
        case mode_gain_zero:
            break;
        case mode_ping:
            err = ping_leg(ctx, address);
            if(err<0)
            {
                logm(SL4C_ERROR, "Unable to communicate with leg address 0x%02x: %s\n",
                     address, modbus_strerror(errno));
            }
            else if(err == 0)
            {
                logm(SL4C_WARNING, "Leg address 0x%02x returned unexpected ping value.\n",
                     address);
            }
            else
            {
                logm(SL4C_INFO, "Leg address 0x%02x ping->ready.", address);
                newmode = mode_ready;
            }
            break;
        case mode_ready:
            break;
    }
    return newmode;
}

static void setall_leg_mode(enum leg_control_mode *leg_mode, int nlegs, enum leg_control_mode m)
{
    for(int i=0; i<nlegs; i++)
        leg_mode[i] = m;
}

static bool all_leg_mode(enum leg_control_mode* leg_mode, int nlegs, enum leg_control_mode m)
{
    bool ready = true;
    for(int i=0; i<nlegs; i++)
        ready &= leg_mode[i] == m;
    return ready;
}

static int count_leg_mode(enum leg_control_mode* leg_mode, int nlegs, enum leg_control_mode m)
{
    int c=0;
    for(int l=0;l<nlegs;l++)
        if(leg_mode[l] == m) c++;
    return c;
}

static int set_gait(struct leg_thread_state* state, char *gaitname)
{
    int g;
    for(g=0; g<state->ngaits; g++)
        if(strcmp(state->gaits[g].name, gaitname) == 0)
        {
            state->current_gait = g;
            break;
        }
    if(g==state->ngaits)
    {
        logm(SL4C_WARNING, "Could not find gait \"%s\", using index %d = \"%s\"",
                gaitname, state->current_gait, state->gaits[state->current_gait].name);
        return -1;
    }
    else
    {
        logm(SL4C_INFO, "Using gait index %d = \"%s\"",
                state->current_gait, state->gaits[state->current_gait].name);
        return 0;
    }
}

static void run_leg_thread_once(struct leg_thread_state* state, struct leg_control_parameters *parameters, float dt)
{
    int count;

    switch(state->legs_mode)
    {
        case mode_all_init:
            setall_leg_mode(state->leg_mode, state->nlegs, mode_ping);
            logm(SL4C_INFO, "all_init->all_ping.");
            state->legs_mode = mode_all_ping;
            break;
        case mode_all_ping:
            if(all_leg_mode(state->leg_mode, state->nlegs, mode_ready))
                switch(parameters->enable)
                {
                    case ENABLE_WALK:
                        logm(SL4C_INFO, "all_ping->all_pos_enable.");
                        setall_leg_mode(state->leg_mode, state->nlegs, mode_set_position);
                        state->legs_mode = mode_all_pos_enable;
                        break;
                    case ENABLE_DISABLE:
                        logm(SL4C_INFO, "all_ping->all_pos_disable.");
                        setall_leg_mode(state->leg_mode, state->nlegs, mode_set_position);
                        state->legs_mode = mode_all_pos_disable;
                        break;
                }
            break;
        case mode_all_pos_disable:
            if(all_leg_mode(state->leg_mode, state->nlegs, mode_error_zero))
                switch(parameters->lock)
                {
                    case LOCK_LOCK:
                        logm(SL4C_INFO, "all_pos_disable->all_gain_zero.");
                        setall_leg_mode(state->leg_mode, state->nlegs, mode_set_gain_zero);
                        state->legs_mode = mode_all_gain_zero;
                        break;
                    case LOCK_FREE:
                        logm(SL4C_INFO, "all_pos_disable->mode_all_gain_operational_free.");
                        setall_leg_mode(state->leg_mode, state->nlegs, mode_set_gain_operational);
                        state->legs_mode = mode_all_gain_operational_free;
                        break;
                }
            break;
        case mode_all_gain_zero:
            if(all_leg_mode(state->leg_mode, state->nlegs, mode_gain_zero))
            {
                logm(SL4C_INFO, "all_gain_zero->all_air_vent_lock.");
                state->legs_mode = mode_all_air_vent_lock;
            } else if((parameters->enable == ENABLE_WALK) ||
                      (parameters->lock == LOCK_FREE))
            {
                logm(SL4C_INFO, "all_gain_zero->all_ping.");
                setall_leg_mode(state->leg_mode, state->nlegs, mode_ping);
                state->legs_mode = mode_all_ping;
            }
            break;
        case mode_all_air_vent_lock:
            air_vent();
            if(parameters->enable == ENABLE_WALK)
            {
                logm(SL4C_INFO, "all_air_vent_lock->all_ping.");
                setall_leg_mode(state->leg_mode, state->nlegs, mode_ping);
                state->legs_mode = mode_all_ping;
            }
            else if(parameters->lock == LOCK_FREE)
            {
                logm(SL4C_INFO, "all_air_vent_lock->all_pos_disable.");
                setall_leg_mode(state->leg_mode, state->nlegs, mode_set_position);
                state->legs_mode = mode_all_pos_disable;
            }
            break;
        case mode_all_gain_operational_free:
            if(all_leg_mode(state->leg_mode, state->nlegs, mode_gain_operational))
            {
                logm(SL4C_INFO, "all_gain_operational_free->all_air_vent_free.");
                state->legs_mode = mode_all_air_vent_free;
            }
            else if((parameters->enable == ENABLE_WALK) ||
                      (parameters->lock == LOCK_LOCK))
            {
                logm(SL4C_INFO, "all_gain_operational_free->all_ping.");
                setall_leg_mode(state->leg_mode, state->nlegs, mode_ping);
                state->legs_mode = mode_all_ping;
            }
            break;
        case mode_all_air_vent_free:
            air_vent();
            if(parameters->enable == ENABLE_WALK)
            {
                logm(SL4C_INFO, "all_air_vent_free->all_ping.");
                setall_leg_mode(state->leg_mode, state->nlegs, mode_ping);
                state->legs_mode = mode_all_ping;
            }
            else if(parameters->lock == LOCK_LOCK)
            {
                logm(SL4C_INFO, "all_air_vent_free->all_gain_zero.");
                setall_leg_mode(state->leg_mode, state->nlegs, mode_set_gain_zero);
                state->legs_mode = mode_all_gain_zero;
            }
            break;
        case mode_all_pos_enable:
            if(all_leg_mode(state->leg_mode, state->nlegs, mode_error_zero))
            {
                logm(SL4C_INFO, "all_air_pos_enable->all_air_on.");
                state->legs_mode = mode_all_air_on;
            }
            else if(parameters->enable == ENABLE_DISABLE)
            {
                logm(SL4C_INFO, "all_pos_enable->all_ping.");
                setall_leg_mode(state->leg_mode, state->nlegs, mode_ping);
                state->legs_mode = mode_all_ping;
            }
            break;
        case mode_all_air_on:
            air_on();
            logm(SL4C_INFO, "all_air_on->all_gain_operational_walk.");
            setall_leg_mode(state->leg_mode, state->nlegs, mode_set_gain_operational);
            state->legs_mode = mode_all_gain_operational_walk;
            break;
        case mode_all_gain_operational_walk:
            if(all_leg_mode(state->leg_mode, state->nlegs, mode_gain_operational))
            {
                logm(SL4C_INFO, "all_gain_operational_walk->all_walk.");
                setall_leg_mode(state->leg_mode, state->nlegs, mode_move_start);
                char *gaitname = state->gait_selections[parameters->gait_selection];
                set_gait(state, gaitname);
                logm(SL4C_INFO, "Selected gait %s.", gaitname);
                state->legs_mode = mode_all_walk;
            }
            else if(parameters->enable == ENABLE_DISABLE)
            {
                logm(SL4C_INFO, "all_gain_operational_walk->all_ping.");
                setall_leg_mode(state->leg_mode, state->nlegs, mode_ping);
                state->legs_mode = mode_all_ping;
            }
            break;
        case mode_all_walk:
            if(parameters->enable == ENABLE_DISABLE)
            {
                logm(SL4C_INFO, "all_walk->all_ping.");
                setall_leg_mode(state->leg_mode, state->nlegs, mode_ping);
                state->legs_mode = mode_all_ping;
            }
            count = count_leg_mode(state->leg_mode, state->nlegs, mode_move_start);
            if(count > 0)
            {
                logm(SL4C_INFO, "moving %d", count);
            }
            char *gaitname = state->gait_selections[parameters->gait_selection];
            if(strcmp(gaitname, state->gaits[state->current_gait].name) != 0)
            {
                set_gait(state, gaitname);
                logm(SL4C_INFO, "Selected gait %s.", gaitname);
            }
            break;
    }
    if(state->legs_mode == mode_all_walk)
    {
        compute_toe_positions(state, parameters, dt);
    }
    for(int leg=0; leg<state->nlegs; leg++)
    {
       state->leg_mode[leg] = run_leg_state_machine(state->leg_mode[leg],
                                                    state->legs[leg].address,
                                                    state->joint_gains,
                                                    &state->toe_position_measured[leg],
                                                    state->valid_measurements,
                                                    &state->commanded_toe_positions[leg],
                                                    state->toe_position_tolerance,
                                                    state->ctx);
    }
}

static int get_measured_positions(struct leg_thread_state* state)
{
    int err, ret=0;
    float pressure[state->nlegs][6];
    for(int leg=0;leg<state->nlegs;leg++)
    {
        err = get_toe_feedback(state->ctx, state->legs[leg].address, &(state->toe_position_measured[leg]), &(pressure[leg]));
        if(err)
        {
            ret = err;
            continue;
        }
        for(int joint=0;joint<JOINT_COUNT;joint++)
        {
            state->base_end_pressure[leg][joint] = pressure[leg][2*joint];
            state->rod_end_pressure[leg][joint] = pressure[leg][2*joint + 1];
        }
    }
    return ret;
}

static int send_telemetry(struct leg_thread_state* state)
{
    ssize_t offset = ringbuf_acquire(state->definition->telemetry_queue->ringbuf, state->telemetry_worker, sizeof(stomp_telemetry_leg));
    if(offset >= 0)
    {
        stomp_telemetry_leg *telem = (stomp_telemetry_leg *)(state->definition->telemetry_queue->buffer + offset);
        for(int leg=0;leg<state->nlegs;leg++)
        {
            for(int joint=0;joint<JOINT_COUNT;joint++)
            {
                telem->base_end_pressure[JOINT_COUNT * leg + joint] = state->base_end_pressure[leg][joint];
                telem->rod_end_pressure[JOINT_COUNT * leg + joint] = state->rod_end_pressure[leg][joint];
            }
            telem->toe_position_measured_X[leg] = state->toe_position_measured[leg][0];
            telem->toe_position_measured_Y[leg] = state->toe_position_measured[leg][1];
            telem->toe_position_measured_Z[leg] = state->toe_position_measured[leg][2];
            telem->toe_position_commanded_X[leg] = state->commanded_toe_positions[leg][0];
            telem->toe_position_commanded_Y[leg] = state->commanded_toe_positions[leg][1];
            telem->toe_position_commanded_Z[leg] = state->commanded_toe_positions[leg][2];
        }
        telem->observed_period = state->observed_period;
        ringbuf_produce(state->definition->telemetry_queue->ringbuf, state->telemetry_worker);
    }
    return 0;
}

static int check_command_queue(struct leg_thread_state* state)
{
    size_t offset;
    size_t s = ringbuf_consume(state->definition->command_queue->ringbuf, &offset);
    if(s >= sizeof(stomp_modbus))
    {
        size_t rs = s;
        while(s >= sizeof(stomp_modbus))
        {
            stomp_modbus* request = (stomp_modbus*)(state->definition->command_queue->buffer + offset);
            switch(request->command)
            {
                //TODO:
                //write_register
                //write_registers
                //write_bits
                //read_registers
                //read_input_registers
                //read_bits
            }
            ssize_t offset = ringbuf_acquire(state->definition->response_queue->ringbuf, state->response_worker, sizeof(stomp_modbus));
            if(offset >= 0)
            {
                stomp_modbus *response = (stomp_modbus *)(state->definition->response_queue->buffer + offset);
                memcpy(response, request, sizeof(stomp_modbus));
                ringbuf_produce(state->definition->response_queue->ringbuf, state->response_worker);
            }
            s -= sizeof(stomp_modbus);
            offset += sizeof(stomp_modbus);
        }
        ringbuf_release(state->definition->command_queue->ringbuf, rs);
    }
    return 0;
}

static void *run_leg_thread(void *ptr)
{
    struct leg_thread_state *state = (struct leg_thread_state*)ptr;
    state->telemetry_worker = ringbuf_register(state->definition->telemetry_queue->ringbuf, 0);
    if(!state->telemetry_worker)
    {
        logm(SL4C_FATAL, "Unable to register worker for telemetry queue\n");
        return (void *)-1;
    }

    state->response_worker = ringbuf_register(state->definition->response_queue->ringbuf, 0);
    if(!state->telemetry_worker)
    {
        logm(SL4C_FATAL, "Unable to register worker for command response queue\n");
        return (void *)-1;
    }

    get_float(state->definition->config, "forward_deadband", 0.05f, &(state->forward_deadband));
    get_float(state->definition->config, "angular_deadband", 0.05f,  &(state->angular_deadband));
    get_float(state->definition->config, "min_angle_change_velocity", 0.005f,  &(state->min_angle_change_velocity));

    toml_table_t *legs_config = toml_table_in(state->definition->config,
                                             "legs");
    state->legs = parse_leg_descriptions(legs_config, &state->nlegs);
    state->joint_gains = parse_joint_gains(legs_config);
    state->toe_position_measured = malloc(sizeof(float[state->nlegs][3]));
    state->commanded_toe_positions = malloc(sizeof(float[state->nlegs][3]));
    state->initial_toe_positions = malloc(sizeof(float[state->nlegs][3]));
    state->base_end_pressure = malloc(sizeof(float[state->nlegs][3]));
    state->rod_end_pressure = malloc(sizeof(float[state->nlegs][3]));
    state->leg_scale = malloc(sizeof(float[state->nlegs][3]));
    for(int i=0;i<state->nlegs;i++)
        for(int j=0;j<3;j++)
            state->leg_scale[i][j] = 1.0f;
    state->leg_offset = malloc(sizeof(float[state->nlegs][3]));
    for(int i=0;i<state->nlegs;i++)
        for(int j=0;j<3;j++)
            state->leg_offset[i][j] = 0.0f;
    state->leg_mode = malloc(state->nlegs * sizeof(enum leg_control_mode));

    get_float(legs_config, "toe_position_tolerance", 0.030, &state->toe_position_tolerance);
    get_float(legs_config, "telemetry_frequency", 30.0f, &state->telemetry_frequency);
    get_float(legs_config, "telemetry_period_smoothing", 0.5, &state->telemetry_period_smoothing);
    get_float(legs_config, "support_pressure", 5.0f, &(state->support_pressure));
    get_float(legs_config, "desired_ride_height", 0.230, &(state->desired_ride_height));
    get_float(legs_config, "ride_height_igain", 0.0f, &(state->ride_height_igain));
    get_float(legs_config, "ride_height_pgain", 0.0f, &(state->ride_height_pgain));
    get_float(legs_config, "ride_height_scale", 0.0f, &(state->ride_height_scale));
    get_float(legs_config, "ride_height_integrator_limit", 0.0f, &(state->ride_height_integrator_limit));

    state->ride_height_integrator = calloc(state->nlegs, sizeof(float));

    state->steps = parse_steps(state->definition->config, &state->nsteps);

    state->gaits = parse_gaits(state->definition->config, &state->ngaits, state->steps, state->nsteps);

    toml_raw_t tomlr = toml_raw_in(state->definition->config, "initial_gait");
    char *initial_gait;
    toml_rtos(tomlr, &initial_gait);
    set_gait(state, initial_gait);

    state->gait_selections = parse_gait_selections(state->definition->config, state->gaits, state->ngaits, initial_gait);

    toml_table_t *geometry = toml_table_in(state->definition->config, "geometry");
    get_float(geometry, "halfwidth", 0.0600f, &state->turning_width);

    struct leg_control_parameters parameters = {
        .forward_velocity = 0.0f,
        .angular_velocity = 0.0f,
        .enable = ENABLE_DISABLE,
        .lock = LOCK_FREE
    };
    bool loop_phase = false;

    state->legs_mode = mode_all_init;
    state->timer = create_rate_timer(state->definition->frequency);
    logm(SL4C_DEBUG, "Create timer with frequency %f",state->definition->frequency);
    float elapsed = 0, dt=0;;
    float telem_interval = 0;
    while(state->shouldrun)
    {
        if(loop_phase)
        {
            read_parameters(state->definition->parameter_queue, &parameters);
            run_leg_thread_once(state, &parameters, dt);
        } else {
            state->valid_measurements = get_measured_positions(state) == 0;
        }
        telem_interval += dt;
        if(state->valid_measurements && (telem_interval > 1.0/state->telemetry_frequency))
        {
            telem_interval = 0;
            send_telemetry(state);
        }

        check_command_queue(state);

        state->observed_period *= state->telemetry_period_smoothing;
        state->observed_period += (1.0f - state->telemetry_period_smoothing) * dt;
        sleep_rate(state->timer, &elapsed, &dt);
        logm(SL4C_FINER, "lp: %s el: %6.3f dt: %5.3f", loop_phase ? "walk" : "tlem", elapsed, dt);
        loop_phase ^= true;
    }
    destroy_rate_timer(&state->timer);
    free_leg_description(state->legs, state->nlegs);
    free(state->joint_gains);
    free(state->leg_mode);
    free(state->initial_toe_positions);
    free(state->commanded_toe_positions);
    free(state->toe_position_measured);
    free(state->initial_toe_positions);
    free_step_descriptions(state->steps, state->nsteps);
    free_gait_descriptions(state->gaits, state->ngaits);
    return NULL;
}

struct leg_thread_state* create_leg_thread(struct leg_thread_definition *leg_thread, const char * progname)
{
    struct leg_thread_state *state = malloc(sizeof(struct leg_thread_state));
    state->definition = leg_thread;
    if(create_modbus_interface(leg_thread->devname, leg_thread->baud, leg_thread->byte_timeout, leg_thread->response_timeout, &state->ctx))
    {
        free(state);
        return 0;
    }

    struct sched_param sched = {
        .sched_priority = 10
    };
    if(sched_setscheduler(0, SCHED_FIFO, &sched) != 0)
    {
        perror("Unable to set scheduler");
        logm(SL4C_FATAL, "Try:\nsudo setcap \"cap_sys_nice=ep\" %s\n", progname);
        free(state);
        return 0;
    }

    state->shouldrun = true;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&state->thread, &attr, &run_leg_thread, state);
    return state;
}

void terminate_leg_thread(struct leg_thread_state **state)
{
    (*state)->shouldrun = false;
    void *res;
    pthread_join((*state)->thread, &res);
    logm(SL4C_INFO, "run_leg_thread=%zu\n", (size_t)res);
    free(*state);
    *state = NULL;
}
