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
    struct gait *gaits;
    int current_gait;
    float position_ramp_time;
    float toe_position_tolerance;
    float telemetry_frequency;
    float telemetry_period_smoothing;
    float forward_deadband;
    float angular_deadband;
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
    float *leg_scale;
    float turning_width;
    float walk_phase;
    float observed_period;
};


static int set_gain(modbus_t *ctx, uint8_t address, struct joint_gains *gain)
{
    uint32_t sec, saved_timeout;
    modbus_get_response_timeout(ctx, &sec, &saved_timeout);
    modbus_set_response_timeout(ctx, 0, 100000);
    int err = set_servo_gains(ctx, address, &gain->proportional_gain, &gain->derivative_gain, &gain->force_damping);
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

static int compute_leg_position(struct leg_thread_state* state, int leg_index, float phase, float scale, float (*toe_position)[3])
{
    struct gait *gait = (state->gaits+state->current_gait);
    struct step *step = (state->steps+gait->step_index);
    float discard;
    float leg_phase = modff(phase + gait->phase_offsets[leg_index], &discard);
    int index = find_interpolation_index(step->phase, step->npoints, leg_phase);
    if(index<0)
        return index;
    interpolate_value(step->phase, step->X, step->npoints, index, leg_phase, &((*toe_position)[0]));
    interpolate_value(step->phase, step->Y, step->npoints, index, leg_phase, &((*toe_position)[1]));
    interpolate_value(step->phase, step->Z, step->npoints, index, leg_phase, &((*toe_position)[2]));
    // TODO Make this correct for angles other than 0, 180.
    (*toe_position)[1] *= copysignf(1.0f, cosf(state->legs[leg_index].orientation[2] * deg2rad)) * scale;
    return 0;
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

static float deadband(float f, float d)
{
    if(fabsf(f) < d)
        return 0;
    else
        return f - copysignf(d, f);
}

static void compute_walk_velocity(struct leg_thread_state* state, struct leg_control_parameters *p, float* left_velocity, float *right_velocity)
{
    float forward = deadband(p->forward_velocity, state->forward_deadband);
    float angular = deadband(p->angular_velocity, state->angular_deadband);
    *right_velocity = forward + state->turning_width * angular;
    *left_velocity = forward - state->turning_width * angular;
}

static float compute_walk_frequency(float step_length, float left_velocity, float right_velocity)
{
    return MAX(fabsf(left_velocity), fabsf(right_velocity)) / step_length;
}

static bool anyclose(float *x, int n, float y, float dy)
{
    bool close = true;
    for(int i=0;i<n;i++)
        close &= (fabsf(x[i] - y) < dy);
    return close;
}

static int compute_walk_scale(struct leg_thread_state *state, float phase, float left_velocity, float right_velocity, float* leg_scale)
{
    float left_scale, right_scale, scale;
    if(fabsf(right_velocity) <= 1e-4 && fabsf(left_velocity) <= 1e-4)
    {
        left_scale = copysignf(1.0f, left_velocity);
        right_scale = copysignf(1.0f, right_velocity);
    }
    else if(fabsf(right_velocity) <= 1e-4 && fabsf(left_velocity) > 1e-4)
    {
        right_scale = 0.0f;
        left_scale = copysignf(1.0f, left_velocity);
    }
    else
    {
        scale = fabs(left_velocity / right_velocity);
        if(scale < 1.0)
        {
            left_scale = copysignf(scale, left_velocity);
            right_scale = copysignf(1.0f, right_velocity);
        }
        else
        {
            left_scale = copysignf(1.0f, left_velocity);
            right_scale = copysignf(1.0f/scale, right_velocity);
        }
    }
    struct step* step=&state->steps[state->gaits[state->current_gait].step_index];
    if((signbit(leg_scale[0]) == signbit(right_scale)) ||
       anyclose(step->swap_phase, step->nswap, phase, step->swap_tolerance))
        for(int l=0;l<state->nlegs / 2;l++)
        {
            leg_scale[l] = right_scale;
        }

    if((signbit(leg_scale[state->nlegs/2]) == signbit(left_scale)) ||
       anyclose(step->swap_phase, step->nswap, phase, step->swap_tolerance))
        for(int l=state->nlegs/2;l<state->nlegs;l++)
        {
            leg_scale[l] = left_scale;
        }
    return 0;
}

static int compute_toe_positions(struct leg_thread_state* state, struct leg_control_parameters *p, float dt)
{
    float left_velocity, right_velocity;
    compute_walk_velocity(state, p, &left_velocity, &right_velocity);
    float frequency = compute_walk_frequency(state->steps[state->gaits[state->current_gait].step_index].length,
                                             left_velocity, right_velocity);
    float dummy;
    state->walk_phase = MAX(modff(state->walk_phase + frequency * dt, &dummy), 0.0f);
    compute_walk_scale(state, state->walk_phase, left_velocity, right_velocity, state->leg_scale);

    int ret = 0;
    for(int leg=0; leg<state->nlegs; leg++)
    {
        compute_leg_position(state, leg, state->walk_phase, state->leg_scale[leg], &state->commanded_toe_positions[leg]);
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

    get_float(state->definition->config, "forward_deadband", &(state->forward_deadband));
    get_float(state->definition->config, "angular_deadband", &(state->angular_deadband));

    toml_table_t *legs_config = toml_table_in(state->definition->config,
                                             "legs");
    state->legs = parse_leg_descriptions(legs_config, &state->nlegs);
    state->joint_gains = parse_joint_gains(legs_config);
    state->toe_position_measured = malloc(sizeof(float[state->nlegs][3]));
    state->commanded_toe_positions = malloc(sizeof(float[state->nlegs][3]));
    state->initial_toe_positions = malloc(sizeof(float[state->nlegs][3]));
    state->base_end_pressure = malloc(sizeof(float[state->nlegs][3]));
    state->rod_end_pressure = malloc(sizeof(float[state->nlegs][3]));
    state->leg_scale = malloc(sizeof(float[state->nlegs]));
    for(int i=0;i<state->nlegs;i++)
        state->leg_scale[i] = 1.0f;
    state->leg_mode = malloc(state->nlegs * sizeof(enum leg_control_mode));

    get_float(legs_config, "position_ramp_time", &state->position_ramp_time);
    get_float(legs_config, "toe_position_tolerance", &state->toe_position_tolerance);
    get_float(legs_config, "telemetry_frequency", &state->telemetry_frequency);
    get_float(legs_config, "telemetry_period_smoothing", &state->telemetry_period_smoothing);

    state->steps = parse_steps(state->definition->config, &state->nsteps);

    state->gaits = parse_gaits(state->definition->config, &state->ngaits, state->steps, state->nsteps);

    toml_table_t *geometry = toml_table_in(state->definition->config, "geometry");
    get_float(geometry, "halfwidth", &state->turning_width);

    struct leg_control_parameters parameters = {
        .forward_velocity = 0.0f,
        .angular_velocity = 0.0f,
        .enable = ENABLE_DISABLE,
        .lock = LOCK_FREE
    };
    bool loop_phase = false;

    state->legs_mode = mode_all_init;
    state->timer = create_rate_timer(state->definition->frequency);
    logm(SL4C_DEBUG, "Create timer with period %f",state->definition->frequency);
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
        logm(SL4C_FINE, "lp: %s el: %6.3f dt: %5.3f", loop_phase ? "walk" : "tlem", elapsed, dt);
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
